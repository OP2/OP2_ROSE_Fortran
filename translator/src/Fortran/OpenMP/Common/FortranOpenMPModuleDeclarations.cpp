#include <FortranOpenMPModuleDeclarations.h>

FortranOpenMPModuleDeclarations::FortranOpenMPModuleDeclarations (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranModuleDeclarations (parallelLoop, moduleScope)
{
}
