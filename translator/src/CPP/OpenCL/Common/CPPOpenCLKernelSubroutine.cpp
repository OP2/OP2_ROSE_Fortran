#include <CPPOpenCLKernelSubroutine.h>

CPPOpenCLKernelSubroutine::CPPOpenCLKernelSubroutine (
    SgScopeStatement * moduleScope,
    Subroutine <SgFunctionDeclaration> * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
}
