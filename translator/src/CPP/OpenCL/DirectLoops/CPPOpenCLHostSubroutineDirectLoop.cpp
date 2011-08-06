#include <CPPOpenCLHostSubroutineDirectLoop.h>
//#include <CPPTypesBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CPPOpenCLReductionSubroutine.h>
#include <CommonNamespaces.h>

//using namespace SageBuilder;
/*
 * ======================================================
 * Private functions
 * ======================================================
 */


void
CPPOpenCLHostSubroutineDirectLoop::createVariableSizesInitialisationStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating OpenCL kernel prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

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
          moduleDeclarations->getDataSizesVariableDeclaration ()),
          buildOpaqueVarRefExp (variableName, subroutineScope));

      /*
       * ======================================================
       * The size value changes for reduction variables
       * ======================================================
       */

      if (parallelLoop->isReductionRequired (i) == false)
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression, buildVarRefExp (
                dataSizesDeclaration->getFieldDeclarations ()->get (
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
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelInitialisationStatements ()
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
  string const maxFunctionName = "MAX";

  /*
   * ======================================================
   * The following values are copied from Mike Giles'
   * implementation and may be subject to future changes
   * ======================================================
   */
  int const nblocks = 200;
  int const nthreads = 128;

  /*
   * ======================================================
   * nblocks = 200
   * ======================================================
   */
  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get ( OpenCL::CPP::blocksPerGrid) ), 
      buildIntVal (nblocks) );

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * nthread = 128 //TODO: change in order to wrap with OP_BLOCK_SIZE (how?)
   * ======================================================
   */
  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp ( variableDeclarations->get (OpenCL::CPP::threadsPerBlock)),
      buildIntVal (nthreads));

  appendStatement (assignmentStatement2, subroutineScope);
  
  /*
   * ======================================================
   * ntotthread = nblocks * nthread //TODO: check if we can reuse SgExprStatement pointers
   * ======================================================
   */
  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
      buildVarRefExp( variableDeclarations->get(OpenCL::CPP::blocksPerGrid) ),
      buildVarRefExp( variableDeclarations->get(OpenCL::CPP::threadsPerBlock) ) );
  
  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp ( variableDeclarations->get (OpenCL::CPP::totalThreads)),
      multiplyExpression2 );

  appendStatement (assignmentStatement2, subroutineScope);

  /*SgExprStatement * assignmentStatement3 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (DirectLoop::CPP::KernelSubroutine::warpSize)),
      buildOpaqueVarRefExp (OPWarpSizeVariableName, subroutineScope));*/

  appendStatement (assignmentStatement3, subroutineScope);

  /* ======================================================
   * nshared = 0
   * ======================================================
   */
  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OpenCL::CPP::sharedMemorySize)), 
      buildIntVal (0) );

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
         * nshared = MAX(nshared, size * dim)
         * ======================================================
         */

        SgVarRefExp * parameterExpression1 = buildVarRefExp (
            variableDeclarations->get (OpenCL::CPP::sharedMemorySize));

        SgExpression * parameterExpression2 = buildMultiplyOp (
            buildIntVal ( parallelLoop->getOpDatDimension (i)),
            CPPStatementsAndExpressionsBuilder::getCPPKindOfOpDat ( parallelLoop->getOpDatType (i))); //FIXME: sizeof(..)?

        SgExprListExp * actualParameters = buildExprListExp (
            parameterExpression1, parameterExpression2);

        SgFunctionSymbol * maxFunctionSymbol =
            CPPTypesBuilder::buildNewCPPFunction (maxFunctionName, subroutineScope);

        SgFunctionCallExp * maxFunctionCall = buildFunctionCallExp (
            maxFunctionSymbol, actualParameters);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get(OpenCL::CPP::sharedMemorySize)), 
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
      variableDeclarations->get (OpenCL::CPP::sharedMemorySize)),
      buildOpaqueVarRefExp (OPWarpSizeVariableName, subroutineScope));

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock)),
      multiplyExpression5);

  appendStatement (assignmentStatement5, subroutineScope);

  /*
   * ======================================================
   * nshared = nshared * nthreads
   * ======================================================
   */
  SgExpression * multiplyExpression6 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::sharedMemorySize)),
      buildVarRefExp ( variableDeclarations->get (OpenCL::CPP::threadsPerBlock)));

  SgExprStatement * assignmentStatement6 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OpenCL::CPP::sharedMemorySize)), multiplyExpression6);

  appendStatement (assignmentStatement6, subroutineScope);
}

void
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelLocalVariableDeclarationsForDirectLoop ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildIntType;
  using SageBuilder::buildAssignInitializer;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating OpenCL configuration parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration1 = buildVariableDeclaration (
      DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock,
      CPPTypesBuilder::getFourByteInteger (), buildAssignInitializer (
          buildIntVal (0), buildIntType ()), subroutineScope);

  variableDeclarations->add (
      DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock,
      variableDeclaration1);

  variableDeclaration1->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration1, subroutineScope);

  SgVariableDeclaration * variableDeclaration2 = buildVariableDeclaration (
      DirectLoop::CPP::KernelSubroutine::warpSize,
      CPPTypesBuilder::getFourByteInteger (), buildAssignInitializer (
          buildIntVal (0), buildIntType ()), subroutineScope);

  variableDeclarations->add (DirectLoop::CPP::KernelSubroutine::warpSize,
      variableDeclaration2);

  variableDeclaration2->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration2, subroutineScope);
}

void
CPPOpenCLHostSubroutineDirectLoop::createStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildBoolValExp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (true));

  SgBasicBlock * ifBody = createFirstTimeExecutionStatements ();

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (
          moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (false));

  appendStatement (assignmentStatement1, ifBody);

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, subroutineScope);

  createOpenCLKernelInitialisationStatements ();

  appendStatement (createTransferOpDatStatements (), subroutineScope);

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
}

void
CPPOpenCLHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  createDataMarshallingLocalVariableDeclarations ();

  createOpenCLKernelLocalVariableDeclarations ();

  createOpenCLKernelLocalVariableDeclarationsForDirectLoop ();

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

CPPOpenCLHostSubroutineDirectLoop::CPPOpenCLHostSubroutineDirectLoop (
    std::string const & subroutineName, 
    std::string const & userSubroutineName,
    std::string const & kernelSubroutineName,
    CPPParallelLoop * parallelLoop, 
    SgScopeStatement * moduleScope,
    CPPInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
    CPPOpenCLDataSizesDeclarationDirectLoop * dataSizesDeclaration,
    CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
    CPPOpenCLModuleDeclarations * moduleDeclarations) :
  CPPOpenCLHostSubroutine (
      subroutineName, 
      userSubroutineName,
      kernelSubroutineName, 
      parallelLoop, 
      moduleScope,
      initialiseConstantsSubroutine, 
      dataSizesDeclaration,
      opDatDimensionsDeclaration, 
      moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
