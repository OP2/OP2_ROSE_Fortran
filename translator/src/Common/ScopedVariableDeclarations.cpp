#include <ScopedVariableDeclarations.h>
#include <Debug.h>
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
    Debug::getInstance ()->errorMessage ("Unable to find '" + variableName
        + "' in variable declarations", __FILE__, __LINE__);
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
