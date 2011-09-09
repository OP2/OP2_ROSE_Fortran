#include <SubroutineVariableDeclarations.h>
#include <Debug.h>

bool
SubroutineVariableDeclarations::exists (std::string const & variableName)
{
  return theDeclarations.find (variableName) != theDeclarations.end ();
}

SgVariableDeclaration *
SubroutineVariableDeclarations::get (std::string const & variableName)
{
  if (theDeclarations.count (variableName) == 0)
  {
    Debug::getInstance ()->errorMessage ("Unable to find '" + variableName
        + "' in variable declarations", __FILE__, __LINE__);
  }

  return theDeclarations[variableName];
}

void
SubroutineVariableDeclarations::add (std::string const & variableName,
    SgVariableDeclaration * declaration)
{
  theDeclarations[variableName] = declaration;
}

SubroutineVariableDeclarations::SubroutineVariableDeclarations ()
{

}

SubroutineVariableDeclarations::SubroutineVariableDeclarations (std::vector <
    SubroutineVariableDeclarations *> & allDeclarations)
{
  using std::vector;

  for (vector <SubroutineVariableDeclarations *>::iterator it =
      allDeclarations.begin (); it != allDeclarations.end (); ++it)
  {
    SubroutineVariableDeclarations * declarations = *it;

    theDeclarations.insert (declarations->theDeclarations.begin (),
        declarations->theDeclarations.end ());
  }
}
