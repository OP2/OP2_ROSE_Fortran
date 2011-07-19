#include <CPPOpenCLHostSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

CPPOpenCLHostSubroutine::CPPOpenCLHostSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, SgScopeStatement * moduleScope,
    CPPParallelLoop * parallelLoop) :
  CPPHostSubroutine (subroutineName, userSubroutineName, kernelSubroutineName,
      moduleScope, parallelLoop)
{
}
