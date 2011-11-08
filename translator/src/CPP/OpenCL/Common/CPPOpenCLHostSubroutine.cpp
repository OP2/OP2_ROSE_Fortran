#include <CPPOpenCLHostSubroutine.h>
#include <CPPOpenCLKernelSubroutine.h>

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

CPPOpenCLHostSubroutine::CPPOpenCLHostSubroutine (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop)
{
}
