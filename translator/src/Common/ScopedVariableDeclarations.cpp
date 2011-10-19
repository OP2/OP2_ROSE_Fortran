#include <ScopedVariableDeclarations.h>
#include <Exceptions.h>
#include <rose.h>

bool
ScopedVariableDeclarations::exists (std::string const & variableName)
{
  return theDeclarations.find (variableName) != theDeclarations.end ();
}

SgVariableDeclaration *
ScopedVariableDeclarations::get (std::string const & variableName)
{
  if (theDeclarations.count (variableName) == 0)
  {
    throw Exceptions::CodeGeneration::UnknownVariableException (
        "Unable to find '" + variableName + "' in variable declarations");
  }

  return theDeclarations[variableName];
}

void
ScopedVariableDeclarations::add (std::string const & variableName,
    SgVariableDeclaration * declaration)
{
  theDeclarations[variableName] = declaration;
}

ScopedVariableDeclarations::ScopedVariableDeclarations ()
{
}
