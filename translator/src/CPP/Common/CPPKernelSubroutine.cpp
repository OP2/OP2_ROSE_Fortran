#include <CPPKernelSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

CPPKernelSubroutine::CPPKernelSubroutine (std::string const & subroutineName,
    std::string const & userSubroutineName, ParallelLoop * parallelLoop) :
  CPPSubroutine (), KernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop)
{

}
