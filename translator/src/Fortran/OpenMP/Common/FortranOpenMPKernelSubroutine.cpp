#include <FortranOpenMPKernelSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

FortranOpenMPKernelSubroutine::FortranOpenMPKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranReductionSubroutines * reductionSubroutines) :
  FortranKernelSubroutine (subroutineName, userSubroutineName, parallelLoop,
      moduleScope, reductionSubroutines)
{
}
