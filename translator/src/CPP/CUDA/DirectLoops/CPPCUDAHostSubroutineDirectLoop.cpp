#include <CPPCUDAHostSubroutineDirectLoop.h>
#include <CPPParallelLoop.h>
#include <CPPKernelSubroutine.h>

void
CPPCUDAHostSubroutineDirectLoop::createCUDAKernelInitialisationStatements ()
{
}

SgStatement *
CPPCUDAHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
}

void
CPPCUDAHostSubroutineDirectLoop::createStatements ()
{
}

void
CPPCUDAHostSubroutineDirectLoop::createCUDAKernelActualParameterDeclarations ()
{

}

void
CPPCUDAHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  createCUDAConfigurationLaunchDeclarations ();

  createCUDAKernelActualParameterDeclarations ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionDeclarations ();
  }
}

CPPCUDAHostSubroutineDirectLoop::CPPCUDAHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope, CPPCUDAKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPCUDAHostSubroutine (moduleScope, calleeSubroutine, parallelLoop)
{
  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
