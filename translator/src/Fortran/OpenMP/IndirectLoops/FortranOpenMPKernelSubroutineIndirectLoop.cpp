#include <FortranOpenMPKernelSubroutineIndirectLoop.h>

SgStatement *
FortranOpenMPKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{

}

void
FortranOpenMPKernelSubroutineIndirectLoop::createStatements ()
{

}

void
FortranOpenMPKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{

}

void
FortranOpenMPKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{

}

FortranOpenMPKernelSubroutineIndirectLoop::FortranOpenMPKernelSubroutineIndirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranOpenMPKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope)
{
  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
