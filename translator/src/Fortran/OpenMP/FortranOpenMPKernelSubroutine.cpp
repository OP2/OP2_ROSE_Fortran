#include <FortranOpenMPKernelSubroutine.h>

FortranOpenMPKernelSubroutine::FortranOpenMPKernelSubroutine (
    std::string const & subroutineName, Subroutine * userSubroutine,
    ParallelLoop * parallelLoop) :
  Subroutine (subroutineName)
{
  this->userSubroutine = userSubroutine;

  this->parallelLoop = parallelLoop;
}
