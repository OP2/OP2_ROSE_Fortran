#include <FortranOpenMPModuleDeclarations.h>
#include <FortranParallelLoop.h>
#include <ScopedVariableDeclarations.h>
#include <CompilerGeneratedNames.h>
#include <Debug.h>
#include <rose.h>

FortranOpenMPModuleDeclarations::FortranOpenMPModuleDeclarations (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranModuleDeclarations (parallelLoop, moduleScope)
{
}
