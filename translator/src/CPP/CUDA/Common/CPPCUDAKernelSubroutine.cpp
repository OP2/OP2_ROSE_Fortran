#include <CPPCUDAKernelSubroutine.h>
#include <CPPReductionSubroutines.h>
#include <CPPCUDAUserSubroutine.h>

void
CPPCUDAKernelSubroutine::createInitialiseCUDAStageInVariablesStatements ()
{
}

void
CPPCUDAKernelSubroutine::createReductionEpilogueStatements ()
{
}

void
CPPCUDAKernelSubroutine::createCUDAStageInVariablesVariableDeclarations ()
{
}

void
CPPCUDAKernelSubroutine::createCUDASharedVariableDeclarations ()
{
}

CPPCUDAKernelSubroutine::CPPCUDAKernelSubroutine (
    SgScopeStatement * moduleScope, CPPCUDAUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
}
