#include <FortranOpenMPHostSubroutineIndirectLoop.h>

void
FortranOpenMPHostSubroutineIndirectLoop::createStatements ()
{

}

void
FortranOpenMPHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{

}

FortranOpenMPHostSubroutineIndirectLoop::FortranOpenMPHostSubroutineIndirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations) :
  FortranOpenMPHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope)
{
  this->moduleDeclarations = moduleDeclarations;

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
