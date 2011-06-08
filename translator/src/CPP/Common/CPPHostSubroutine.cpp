#include <CPPHostSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

CPPHostSubroutine::CPPHostSubroutine (std::string const & subroutineName,
    std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, ParallelLoop * parallelLoop) :
  CPPSubroutine (), HostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop)
{

}
