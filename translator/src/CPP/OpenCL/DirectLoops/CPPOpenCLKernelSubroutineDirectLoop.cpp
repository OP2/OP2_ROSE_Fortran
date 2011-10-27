#include <CPPOpenCLKernelSubroutineDirectLoop.h>
#include <CPPOpenCLUserSubroutine.h>
#include <CPPParallelLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <Debug.h>

SgStatement *
CPPOpenCLKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
}

SgBasicBlock *
CPPOpenCLKernelSubroutineDirectLoop::createStageInFromDeviceMemoryToLocalThreadVariablesStatements ()
{
}

SgBasicBlock *
CPPOpenCLKernelSubroutineDirectLoop::createStageOutFromLocalThreadVariablesToDeviceMemoryStatements ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createAutoSharedDisplacementInitialisationStatement ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createThreadIDInitialisationStatement ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createInitialiseLocalThreadVariablesStatements ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createStatements ()
{
  createThreadIDInitialisationStatement ();

  createAutoSharedDisplacementInitialisationStatement ();

  createInitialiseLocalThreadVariablesStatements ();

  createExecutionLoopStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
  }
}

void
CPPOpenCLKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  using SageBuilder::buildIntType;

  /*
   * ======================================================
   * OP_DAT formal parameters
   * ======================================================
   */

  createOpDatFormalParameterDeclarations ();
}

CPPOpenCLKernelSubroutineDirectLoop::CPPOpenCLKernelSubroutineDirectLoop (
    SgScopeStatement * moduleScope, CPPOpenCLUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPOpenCLKernelSubroutine (moduleScope, userSubroutine, parallelLoop,
      reductionSubroutines)
{
  Debug::getInstance ()->debugMessage (
      "Creating OpenCL kernel subroutine for direct loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
