#include <CPPCUDAHostSubroutine.h>

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

CPPCUDAHostSubroutine::CPPCUDAHostSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, CPPParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  CPPHostSubroutine (subroutineName, userSubroutineName, kernelSubroutineName,
      parallelLoop, moduleScope)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaHost ();
}
