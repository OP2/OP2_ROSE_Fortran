#include "CPPOpenCLHostSubroutine.h"
#include "CPPOpenCLKernelSubroutine.h"
#include "CPPModuleDeclarations.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "OpenCL.h"
#include "CompilerGeneratedNames.h"
#include "OP2.h"

void
CPPOpenCLHostSubroutine::createReductionEpilogueStatements ()
{
}

void
CPPOpenCLHostSubroutine::createReductionPrologueStatements ()
{
}

void
CPPOpenCLHostSubroutine::createReductionDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace ReductionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));

  variableDeclarations->add (
      getIterationCounterVariableName (2),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2), buildIntType (), subroutineScope));

  variableDeclarations->add (reductionBytes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          reductionBytes, buildIntType (), subroutineScope));

  variableDeclarations->add (reductionSharedMemorySize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          reductionSharedMemorySize, buildIntType (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      string const reductionArrayHostName = getReductionArrayHostName (i);

      Debug::getInstance ()->debugMessage ("Creating host reduction pointer '"
          + reductionArrayHostName + "'", Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgVariableDeclaration * reductionArrayHost =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionArrayHostName, buildPointerType (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope);

      variableDeclarations->add (reductionArrayHostName, reductionArrayHost);
    }
  }
}

void
CPPOpenCLHostSubroutine::createKernelCallEpilogueStatements (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating statements to launch OpenCL kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Assert statement
   * ======================================================
   */

  SgEqualityOp * equalityExpression1 = buildEqualityOp (
      variableDeclarations->getReference (OpenCL::errorCode),
      buildOpaqueVarRefExp (OpenCL::CL_SUCCESS, scope));

  appendStatement (buildExprStatement (
      OpenCL::OP2RuntimeSupport::getAssertMessage (subroutineScope,
          equalityExpression1, buildStringVal (
              "Error setting OpenCL kernel arguments"))), scope);

  /*
   * ======================================================
   * Execute kernel statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode),
      OpenCL::getEnqueueKernelCallExpression (subroutineScope,
          buildOpaqueVarRefExp (OpenCL::commandQueue, scope),
          variableDeclarations->getReference (OpenCL::kernelPointer),
          variableDeclarations->getReference (OpenCL::totalThreadNumber),
          variableDeclarations->getReference (OpenCL::threadsPerBlock),
          variableDeclarations->getReference (OpenCL::event)));

  appendStatement (assignmentStatement1, scope);

  /*
   * ======================================================
   * Assert statement
   * ======================================================
   */

  SgEqualityOp * equalityExpression2 = buildEqualityOp (
      variableDeclarations->getReference (OpenCL::errorCode),
      buildOpaqueVarRefExp (OpenCL::CL_SUCCESS, scope));

  appendStatement (
      buildExprStatement (OpenCL::OP2RuntimeSupport::getAssertMessage (
          subroutineScope, equalityExpression2, buildStringVal (
              "Error executing OpenCL kernel"))), scope);

  /*
   * ======================================================
   * Complete device commands statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode),
      OpenCL::getFinishCommandQueueCallExpression (subroutineScope,
          buildOpaqueVarRefExp (OpenCL::commandQueue, scope)));

  appendStatement (assignmentStatement2, scope);

  /*
   * ======================================================
   * Assert statement
   * ======================================================
   */

  SgEqualityOp * equalityExpression3 = buildEqualityOp (
      variableDeclarations->getReference (OpenCL::errorCode),
      buildOpaqueVarRefExp (OpenCL::CL_SUCCESS, scope));

  appendStatement (buildExprStatement (
      OpenCL::OP2RuntimeSupport::getAssertMessage (subroutineScope,
          equalityExpression3, buildStringVal (
              "Error completing device command queue"))), scope);
}

void
CPPOpenCLHostSubroutine::createOpenCLConfigurationLaunchDeclarations ()
{
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration launch local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (OpenCL::blocksPerGrid,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::blocksPerGrid, OpenCL::getSizeType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::threadsPerBlock,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::threadsPerBlock, OpenCL::getSizeType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::totalThreadNumber,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::totalThreadNumber, OpenCL::getSizeType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::sharedMemorySize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::sharedMemorySize, OpenCL::getSizeType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::errorCode,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::errorCode, OpenCL::getSignedIntegerType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::event,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::event, OpenCL::getEventType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::kernelPointer,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::kernelPointer, OpenCL::getKernelType (subroutineScope),
          subroutineScope));
}

CPPOpenCLHostSubroutine::CPPOpenCLHostSubroutine (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations) :
  CPPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop),
      moduleDeclarations (moduleDeclarations)
{
  variableDeclarations->addVisibilityToSymbolsFromOuterScope (
      moduleDeclarations->getDeclarations ());
}
