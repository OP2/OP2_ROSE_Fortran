#include <FortranOpenMPModuleDeclarationsDirectLoop.h>

FortranOpenMPModuleDeclarationsDirectLoop::FortranOpenMPModuleDeclarationsDirectLoop (
    std::string const & userSubroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranOpenMPModuleDeclarations (userSubroutineName, parallelLoop,
      moduleScope)
{
  createOPDATDeclarations ();
}
