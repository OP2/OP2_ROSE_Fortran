#include <boost/lexical_cast.hpp>
#include <FortranKernelSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

FortranKernelSubroutine::FortranKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    ParallelLoop * parallelLoop) :
  FortranSubroutine (subroutineName + SubroutineNameSuffixes::kernelSuffix)
{
  this->userSubroutineName = userSubroutineName;

  this->parallelLoop = parallelLoop;
}
