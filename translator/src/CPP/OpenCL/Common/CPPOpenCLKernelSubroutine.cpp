#include <CPPOpenCLKernelSubroutine.h>
#include <CPPOpenCLUserSubroutine.h>

CPPOpenCLKernelSubroutine::CPPOpenCLKernelSubroutine (
    SgScopeStatement * moduleScope, CPPOpenCLUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
}
