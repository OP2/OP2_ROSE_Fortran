#include <CPPOpenCLHostSubroutine.h>
#include <CPPOpenCLKernelSubroutine.h>
#include <CPPParallelLoop.h>

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
    CPPOpenCLKernelSubroutine * kernelSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPHostSubroutine (moduleScope, kernelSubroutine, parallelLoop)
{
}
