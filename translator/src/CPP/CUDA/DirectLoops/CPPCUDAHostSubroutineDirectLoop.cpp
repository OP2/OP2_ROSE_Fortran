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
CPPCUDAHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
}

CPPCUDAHostSubroutineDirectLoop::CPPCUDAHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope, CPPCUDAKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop) :
    CPPCUDAHostSubroutine (moduleScope, calleeSubroutine, parallelLoop)
{
}
