#include <FortranOpenMPKernelSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

FortranOpenMPKernelSubroutine::FortranOpenMPKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranKernelSubroutine (subroutineName, userSubroutineName, parallelLoop,
      moduleScope)
{
}