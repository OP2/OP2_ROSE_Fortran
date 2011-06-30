#include <FortranCUDAHostSubroutineDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <ROSEHelper.h>
#include <Debug.h>
#include <FortranCUDAReductionSubroutine.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranCUDAHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::opDatDimensions)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::argsSizes)));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        actualParameters->append_expression (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatDeviceName (i))));
      }
      else
      {
        actualParameters->append_expression (buildVarRefExp (
            variableDeclarations->get (
                ReductionSubroutine::reductionArrayDevice)));
      }
    }
  }

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock)));

  SgExpression * dotExpression = buildDotExp (buildVarRefExp (
      variableDeclarations->get (VariableNames::getOpSetName ())),
      buildOpaqueVarRefExp (CommonVariableNames::size, subroutineScope));

  actualParameters->append_expression (dotExpression);

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::warpSize)));

  if (parallelLoop->isReductionRequired () == true)
  {
    actualParameters->append_expression (
        buildVarRefExp (variableDeclarations->get (
            ReductionSubroutine::sharedMemoryStartOffset)));
  }

  SgExprStatement * callStatement = buildFunctionCallStmt (kernelSubroutineName
      + "<<<" + ROSEHelper::getFirstVariableName (variableDeclarations->get (
      CUDA::Fortran::blocksPerGrid)) + ", " + ROSEHelper::getFirstVariableName (
      variableDeclarations->get (CUDA::Fortran::threadsPerBlock)) + ", "
      + ROSEHelper::getFirstVariableName (variableDeclarations->get (
          CUDA::Fortran::sharedMemorySize)) + ">>>", buildVoidType (),
      actualParameters, subroutineScope);

  return callStatement;
}

void
FortranCUDAHostSubroutineDirectLoop::createVariableSizesInitialisationStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  using std::string;

  /*
   * ======================================================
   * In direct loops, sizes are only related to OP_DAT
   * variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = VariableNames::getOpDatSizeName (i);

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::argsSizes)),
          buildOpaqueVarRefExp (variableName, subroutineScope));

      /*
       * ======================================================
       * The size value changes for reduction variables
       * ======================================================
       */

      if (parallelLoop->isReductionRequired (i) == false)
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression, buildVarRefExp (variableDeclarations->get (
                variableName)));

        appendStatement (assignmentStatement, subroutineScope);
      }
      else
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression, buildVarRefExp (variableDeclarations->get (
                ReductionSubroutine::numberOfThreadItems)));

        appendStatement (assignmentStatement, subroutineScope);
      }
    }
  }
}

void
FortranCUDAHostSubroutineDirectLoop::createCUDAKernelInitialisationStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageInterface::appendStatement;
  using std::string;

  string const OPWarpSizeVariableName = "OP_WARP_SIZE";
  string const maxFunctionName = "max";

  /*
   * ======================================================
   * The following values are copied from Mike Giles'
   * implementation and may be subject to future changes
   * ======================================================
   */
  int const nblocks = 200;
  int const nthreads = 128;

  SgExprStatement * assignmentStatement1 =
      buildAssignStatement (buildVarRefExp (variableDeclarations->get (
          CUDA::Fortran::blocksPerGrid)), buildIntVal (nblocks));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (
          variableDeclarations->get (CUDA::Fortran::threadsPerBlock)),
      buildIntVal (nthreads));

  appendStatement (assignmentStatement2, subroutineScope);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::warpSize)),
      buildOpaqueVarRefExp (OPWarpSizeVariableName, subroutineScope));

  appendStatement (assignmentStatement3, subroutineScope);

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          CUDA::Fortran::sharedMemorySize)), buildIntVal (0));

  appendStatement (assignmentStatement4, subroutineScope);

  /*
   * ======================================================
   * Computing value for nshared: an input OP_DAT is copied
   * to shared memory only if its dimension is larger than
   * 1 or it is not encapsulating global data.
   * Therefore, the maximum size of shared memory is
   * equal to maximum size * dimension OP_DAT copied on
   * shared memory, multiplied by the number of threads
   * ======================================================
   */
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->getOpDatDimension (i) > 1
          && parallelLoop->getOpMapValue (i) != GLOBAL)
      {
        /*
         * ======================================================
         * nshared = max (nshared, size * dim)
         * ======================================================
         */

        SgVarRefExp * parameterExpression1 = buildVarRefExp (
            variableDeclarations->get (CUDA::Fortran::sharedMemorySize));

        SgExpression * parameterExpression2 = buildMultiplyOp (buildIntVal (
            parallelLoop->getOpDatDimension (i)),
            FortranStatementsAndExpressionsBuilder::getFortranKindOfOpDat (
                parallelLoop->getOpDatType (i)));

        SgExprListExp * actualParameters = buildExprListExp (
            parameterExpression1, parameterExpression2);

        SgFunctionSymbol * maxFunctionSymbol =
            FortranTypesBuilder::buildNewFortranFunction (maxFunctionName,
                subroutineScope);

        SgFunctionCallExp * maxFunctionCall = buildFunctionCallExp (
            maxFunctionSymbol, actualParameters);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                CUDA::Fortran::sharedMemorySize)), maxFunctionCall);

        appendStatement (assignmentStatement, subroutineScope);
      }
    }
  }

  /*
   * ======================================================
   * offsetS = nshared * OP_WARP_SIZE
   * ======================================================
   */

  SgMultiplyOp * multiplyExpression5 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::sharedMemorySize)),
      buildOpaqueVarRefExp (OPWarpSizeVariableName, subroutineScope));

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock)),
      multiplyExpression5);

  appendStatement (assignmentStatement5, subroutineScope);

  /*
   * ======================================================
   * nshared = nshared * nthreads
   * ======================================================
   */
  SgExpression * multiplyExpression6 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::sharedMemorySize)),
      buildVarRefExp (
          variableDeclarations->get (CUDA::Fortran::threadsPerBlock)));

  SgExprStatement * assignmentStatement6 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          CUDA::Fortran::sharedMemorySize)), multiplyExpression6);

  appendStatement (assignmentStatement6, subroutineScope);
}

void
FortranCUDAHostSubroutineDirectLoop::createCUDAKernelLocalVariableDeclarationsForDirectLoop ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildIntType;
  using SageBuilder::buildAssignInitializer;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration parameters", 2);

  SgVariableDeclaration * variableDeclaration1 = buildVariableDeclaration (
      DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock,
      FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
          buildIntVal (0), buildIntType ()), subroutineScope);

  variableDeclarations->add (
      DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock,
      variableDeclaration1);

  variableDeclaration1->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration1, subroutineScope);

  SgVariableDeclaration * variableDeclaration2 = buildVariableDeclaration (
      DirectLoop::Fortran::KernelSubroutine::warpSize,
      FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
          buildIntVal (0), buildIntType ()), subroutineScope);

  variableDeclarations->add (DirectLoop::Fortran::KernelSubroutine::warpSize,
      variableDeclaration2);

  variableDeclaration2->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration2, subroutineScope);
}

void
FortranCUDAHostSubroutineDirectLoop::createStatements ()
{
  using SageInterface::appendStatement;

  createOpDatDimensionInitialisationStatements ();

  createCUDAKernelInitialisationStatements ();

  createTransferOpDatStatements (subroutineScope);

  createCUDAKernelPrologueStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionPrologueStatements ();
  }

  createVariableSizesInitialisationStatements ();

  appendStatement (createInitialiseConstantsCallStatement (), subroutineScope);

  appendStatement (createKernelFunctionCallStatement (), subroutineScope);

  appendStatement (createThreadSynchroniseCallStatement (), subroutineScope);

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionEpilogueStatements ();
  }

  createCUDAKernelEpilogueStatements ();
}

void
FortranCUDAHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  createOpDatSizesDeclaration (dataSizesDeclarationOfDirectLoop->getType ());

  createOpDatDimensionsDeclaration (opDatDimensionsDeclaration->getType ());

  createDataMarshallingLocalVariableDeclarations ();

  createCUDAKernelLocalVariableDeclarations ();

  createCUDAKernelLocalVariableDeclarationsForDirectLoop ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionLocalVariableDeclarations ();
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranCUDAHostSubroutineDirectLoop::FortranCUDAHostSubroutineDirectLoop (
    std::string const & subroutineName,
    std::string const & userSubroutineName,
    std::string const & kernelSubroutineName,
    FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
    FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclarationOfDirectLoop,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  FortranCUDAHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope,
      initialiseConstantsSubroutine, opDatDimensionsDeclaration),
      dataSizesDeclarationOfDirectLoop (dataSizesDeclarationOfDirectLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", 2);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
