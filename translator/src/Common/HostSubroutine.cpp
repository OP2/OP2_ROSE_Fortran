#include <HostSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

HostSubroutine::HostSubroutine (std::string const & subroutineName,
    std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, ParallelLoop * parallelLoop) :
  Subroutine (subroutineName + "_host"),
      userSubroutineName (userSubroutineName), kernelSubroutineName (
          kernelSubroutineName), parallelLoop (parallelLoop)
{
}
