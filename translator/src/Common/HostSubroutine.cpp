#include <HostSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

HostSubroutine::HostSubroutine (std::string const & subroutineName,
    std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, ParallelLoop * parallelLoop) :
  Subroutine (subroutineName + "_host")
{
  this->userSubroutineName = userSubroutineName;

  this->kernelSubroutineName = kernelSubroutineName;

  this->parallelLoop = parallelLoop;
}
