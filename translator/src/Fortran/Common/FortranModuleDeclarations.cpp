#include <FortranModuleDeclarations.h>
#include <ScopedVariableDeclarations.h>

ScopedVariableDeclarations *
FortranModuleDeclarations::getDeclarations ()
{
  return variableDeclarations;
}

FortranModuleDeclarations::FortranModuleDeclarations (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  parallelLoop (parallelLoop), moduleScope (moduleScope)
{
  variableDeclarations = new ScopedVariableDeclarations ();
}
