#include <CPPCUDAHostSubroutineIndirectLoop.h>
#include <CPPModuleDeclarations.h>

SgStatement *
CPPCUDAHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
}

void
CPPCUDAHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
}

void
CPPCUDAHostSubroutineIndirectLoop::createExecutionPlanDeclarations ()
{
}

void
CPPCUDAHostSubroutineIndirectLoop::createStatements ()
{

}

void
CPPCUDAHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{

}

CPPCUDAHostSubroutineIndirectLoop::CPPCUDAHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope, CPPCUDAKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations) :
  CPPCUDAHostSubroutine (moduleScope, calleeSubroutine, parallelLoop,
      moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
