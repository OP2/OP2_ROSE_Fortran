#include <FortranCUDAHostSubroutineDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <ROSEHelper.h>
#include <Debug.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranCUDAHostSubroutineDirectLoop::createStatementToCallKernelFunction ()
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (
      buildVarRefExp (
          variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        actualParameters->append_expression (buildVarRefExp (
            variableDeclarations[VariableNames::getOpDatDeviceName (i)]));
      }
      else
      {
        actualParameters->append_expression (
            buildVarRefExp (
                variableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayDevice]));
      }
    }
  }

  actualParameters->append_expression (
      buildVarRefExp (
          variableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]));

  SgExpression * dotExpression =
      buildDotExp (buildVarRefExp (
          variableDeclarations[VariableNames::getOpSetName ()]),
          buildOpaqueVarRefExp (
              IndirectAndDirectLoop::Fortran::HostSubroutine::size,
              subroutineScope));

  actualParameters->append_expression (dotExpression);

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]));

  if (parallelLoop->isReductionRequired () == true)
  {
    actualParameters->append_expression (
        buildVarRefExp (
            variableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset]));
  }

  SgExprStatement * callStatement = buildFunctionCallStmt (kernelSubroutineName
      + "<<<" + ROSEHelper::getFirstVariableName (
      variableDeclarations[CUDA::Fortran::VariableNames::blocksPerGrid]) + ", "
      + ROSEHelper::getFirstVariableName (
          variableDeclarations[CUDA::Fortran::VariableNames::threadsPerBlock])
      + ", " + ROSEHelper::getFirstVariableName (
      variableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize])
      + ">>>", buildVoidType (), actualParameters, subroutineScope);

  return callStatement;
}

void
FortranCUDAHostSubroutineDirectLoop::initialiseDeviceVariablesSizesVariable ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
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

      SgExpression
          * sizeVariableField =
              buildDotExp (
                  buildVarRefExp (
                      variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]),
                  buildOpaqueVarRefExp (variableName, subroutineScope));

      SgVarRefExp * varRefExpression = NULL;

      /*
       * ======================================================
       * The size value changes for reduction variables
       * ======================================================
       */

      if (parallelLoop->isReductionRequired (i) == false)
      {
        varRefExpression = buildVarRefExp (variableDeclarations[variableName]);
      }
      else
      {
        varRefExpression
            = buildVarRefExp (
                variableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::numberOfThreadItems]);
      }

      SgExpression * assignExpression = buildAssignOp (sizeVariableField,
          varRefExpression);

      appendStatement (buildExprStatement (assignExpression), subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutineDirectLoop::initialiseAllCUDAVariables ()
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

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (
          variableDeclarations[CUDA::Fortran::VariableNames::blocksPerGrid]),
      buildIntVal (CUDA::Fortran::DirectLoop::nblocks));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (
          variableDeclarations[CUDA::Fortran::VariableNames::threadsPerBlock]),
      buildIntVal (CUDA::Fortran::DirectLoop::nthreads));

  appendStatement (assignmentStatement2, subroutineScope);

  SgExprStatement
      * assignmentStatement3 =
          buildAssignStatement (
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]),
              buildOpaqueVarRefExp (OPWarpSizeVariableName, subroutineScope));

  appendStatement (assignmentStatement3, subroutineScope);

  SgExprStatement
      * assignmentStatement4 =
          buildAssignStatement (
              buildVarRefExp (
                  variableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]),
              buildIntVal (0));

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

        SgVarRefExp
            * parameterExpression1 =
                buildVarRefExp (
                    variableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]);

        SgExpression * parameterExpression2 = buildMultiplyOp (buildIntVal (
            parallelLoop->getOpDatDimension (i)),
            FortranStatementsAndExpressionsBuilder::getFortranKindOf_OP_DAT (
                parallelLoop->getOpDatType (i)));

        SgExprListExp * actualParameters = buildExprListExp (
            parameterExpression1, parameterExpression2);

        SgFunctionSymbol * maxFunctionSymbol =
            FortranTypesBuilder::buildNewFortranFunction (maxFunctionName,
                subroutineScope);

        SgFunctionCallExp * maxFunctionCall = buildFunctionCallExp (
            maxFunctionSymbol, actualParameters);

        SgExprStatement
            * assignmentStatement =
                buildAssignStatement (
                    buildVarRefExp (
                        variableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]),
                    maxFunctionCall);

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
      variableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]),
      buildOpaqueVarRefExp (OPWarpSizeVariableName, subroutineScope));

  SgExprStatement
      * assignmentStatement5 =
          buildAssignStatement (
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]),
              multiplyExpression5);

  appendStatement (assignmentStatement5, subroutineScope);

  /*
   * ======================================================
   * nshared = nshared * nthreads
   * ======================================================
   */
  SgExpression * multiplyExpression6 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]),
      buildVarRefExp (
          variableDeclarations[CUDA::Fortran::VariableNames::threadsPerBlock]));

  SgExprStatement
      * assignmentStatement6 =
          buildAssignStatement (
              buildVarRefExp (
                  variableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]),
              multiplyExpression6);

  appendStatement (assignmentStatement6, subroutineScope);
}

void
FortranCUDAHostSubroutineDirectLoop::createAdditionalCUDAKernelVariableDeclarations ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildIntType;
  using SageBuilder::buildAssignInitializer;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration parameters", 2);

  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]
      = buildVariableDeclaration (
          DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock,
          FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
              buildIntVal (0), buildIntType ()), subroutineScope);

  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (
      variableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock],
      subroutineScope);

  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]
      = buildVariableDeclaration (
          DirectLoop::Fortran::KernelSubroutine::warpSize,
          FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
              buildIntVal (0), buildIntType ()), subroutineScope);

  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (
      variableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize],
      subroutineScope);
}

void
FortranCUDAHostSubroutineDirectLoop::createStatements ()
{
  using SageInterface::appendStatement;

  initialiseAllCUDAVariables ();

  initialiseDataMarshallingVariables ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createSupportForReductionVariablesBeforeKernel ();
  }

  initialiseDeviceVariablesSizesVariable ();

  appendStatement (createThreadSynchroniseCall (), subroutineScope);

  if (parallelLoop->isReductionRequired () == true)
  {
    createSupportForReductionVariablesAfterKernel ();
  }

  copyDataBackFromDeviceAndDeallocate ();
}

void
FortranCUDAHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::VariableNames::argsSizes,
          dataSizesDeclarationOfDirectLoop->getType (), subroutineScope, 1,
          DEVICE);

  createDataMarshallingDeclarations ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionIterationVariableDeclarations ();
  }

  createCUDAKernelVariableDeclarations ();

  createAdditionalCUDAKernelVariableDeclarations ();

  createReductionVariableDeclarations ();
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
    FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclarationOfDirectLoop,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranCUDAHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", 2);

  this->dataSizesDeclarationOfDirectLoop = dataSizesDeclarationOfDirectLoop;

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
