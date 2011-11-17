#include <ScopedVariableDeclarations.h>
#include <Exceptions.h>
#include <rose.h>

SgVarRefExp *
ScopedVariableDeclarations::getReference (std::string const & variableName)
{
  using namespace SageBuilder;

  if (theDeclarations.count (variableName) == 0)
  {
    throw Exceptions::CodeGeneration::UnknownVariableException (
        "Unable to find '" + variableName + "' in variable declarations");
  }

  return buildVarRefExp (theDeclarations[variableName]);
}

void
ScopedVariableDeclarations::add (std::string const & variableName,
    SgVariableDeclaration * declaration)
{
  if (theDeclarations.count (variableName) != 0)
  {
    throw Exceptions::CodeGeneration::DuplicateVariableException ("'"
        + variableName + "' has already been declared");
  }

  theDeclarations[variableName] = declaration;
}

void
ScopedVariableDeclarations::addVisibilityToSymbolsFromOuterScope (
    ScopedVariableDeclarations * outerScopeDeclarations)
{
  using std::map;
  using std::string;

  for (std::map <string, SgVariableDeclaration *>::iterator it =
      outerScopeDeclarations->theDeclarations.begin (); it
      != outerScopeDeclarations->theDeclarations.end (); ++it)
  {
    add (it->first, it->second);
  }
}

ScopedVariableDeclarations::ScopedVariableDeclarations ()
{
}
