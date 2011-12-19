#include "CPPOpenCLHostSubroutine.h"
#include "CPPOpenCLKernelSubroutine.h"
#include "CPPModuleDeclarations.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "OpenCL.h"

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
      OpenCL::OP2RuntimeSupport::getAssertMessage (scope, equalityExpression1,
          buildStringVal ("Error setting OpenCL kernel arguments"))), scope);

  /*
   * ======================================================
   * Execute kernel statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode),
      OpenCL::getEnqueueKernelCallExpression (scope, buildOpaqueVarRefExp (
          OpenCL::commandQueue, scope), variableDeclarations->getReference (
          OpenCL::kernelPointer), variableDeclarations->getReference (
          OpenCL::totalThreadNumber), variableDeclarations->getReference (
          OpenCL::threadsPerBlock), variableDeclarations->getReference (
          OpenCL::event)));

  appendStatement (assignmentStatement1, scope);

  /*
   * ======================================================
   * Assert statement
   * ======================================================
   */

  SgEqualityOp * equalityExpression2 = buildEqualityOp (
      variableDeclarations->getReference (OpenCL::errorCode),
      buildOpaqueVarRefExp (OpenCL::CL_SUCCESS, scope));

  appendStatement (buildExprStatement (
      OpenCL::OP2RuntimeSupport::getAssertMessage (scope, equalityExpression2,
          buildStringVal ("Error setting OpenCL kernel arguments"))), scope);

  /*
   * ======================================================
   * Complete device commands statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode),
      OpenCL::getFinishCommandQueueCallExpression (scope, buildOpaqueVarRefExp (
          OpenCL::commandQueue, scope)));

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
      OpenCL::OP2RuntimeSupport::getAssertMessage (scope, equalityExpression3,
          buildStringVal ("Error setting OpenCL kernel arguments"))), scope);
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
