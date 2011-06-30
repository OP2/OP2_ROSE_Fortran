#include <FortranOpenMPModuleDeclarationsDirectLoop.h>

FortranOpenMPModuleDeclarationsDirectLoop::FortranOpenMPModuleDeclarationsDirectLoop (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranOpenMPModuleDeclarations (userSubroutineName, parallelLoop,
      moduleScope)
{
  createOpDatDeclarations ();
}
