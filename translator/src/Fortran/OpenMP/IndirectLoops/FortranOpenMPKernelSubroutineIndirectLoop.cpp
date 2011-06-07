#include <FortranOpenMPKernelSubroutineIndirectLoop.h>

void
FortranOpenMPKernelSubroutineIndirectLoop::createStatements ()
{

}

void
FortranOpenMPKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{

}

void
FortranOpenMPKernelSubroutineIndirectLoop::createlocalVariableDeclarations ()
{

}

FortranOpenMPKernelSubroutineIndirectLoop::FortranOpenMPKernelSubroutineIndirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranOpenMPKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope)
{
  createlocalVariableDeclarations();

  createLocalVariableDeclarations();

  createStatements();
}
