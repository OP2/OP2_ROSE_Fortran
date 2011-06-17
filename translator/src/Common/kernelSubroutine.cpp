#include <KernelSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

KernelSubroutine::KernelSubroutine (std::string const & subroutineName,
    std::string const & userSubroutineName, ParallelLoop * parallelLoop) :
  Subroutine (subroutineName + "_kernel"), userSubroutineName (
      userSubroutineName), parallelLoop (parallelLoop)
{
}
