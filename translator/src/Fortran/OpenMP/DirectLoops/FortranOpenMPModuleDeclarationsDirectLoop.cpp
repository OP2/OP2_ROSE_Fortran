#include <FortranOpenMPModuleDeclarationsDirectLoop.h>

FortranOpenMPModuleDeclarationsDirectLoop::FortranOpenMPModuleDeclarationsDirectLoop (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranOpenMPModuleDeclarations (parallelLoop, moduleScope)
{
  createOpDatDeclarations ();
}
