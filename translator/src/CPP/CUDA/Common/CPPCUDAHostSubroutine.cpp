#include <CPPCUDAHostSubroutine.h>
#include <CPPCUDAKernelSubroutine.h>

void
CPPCUDAHostSubroutine::createReductionEpilogueStatements ()
{
}

void
CPPCUDAHostSubroutine::createReductionPrologueStatements ()
{
}

void
CPPCUDAHostSubroutine::createReductionLocalVariableDeclarations ()
{
}

void
CPPCUDAHostSubroutine::createCUDAKernelLocalVariableDeclarations ()
{
}

CPPCUDAHostSubroutine::CPPCUDAHostSubroutine (SgScopeStatement * moduleScope,
    CPPCUDAKernelSubroutine * calleeSubroutine, CPPParallelLoop * parallelLoop) :
  CPPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaHost ();
}
