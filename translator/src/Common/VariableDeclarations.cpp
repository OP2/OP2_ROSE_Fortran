#include <VariableDeclarations.h>

bool
VariableDeclarations::exists (std::string const & variableName)
{
  return theDeclarations.find (variableName) != theDeclarations.end ();
}

SgVariableDeclaration *
VariableDeclarations::get (std::string const & variableName)
{
  return theDeclarations[variableName];
}

void
VariableDeclarations::add (std::string const & variableName,
    SgVariableDeclaration * declaration)
{
  theDeclarations[variableName] = declaration;
}

VariableDeclarations::VariableDeclarations ()
{

}

VariableDeclarations::VariableDeclarations (
    std::vector <VariableDeclarations *> & allDeclarations)
{
  using std::vector;

  for (vector <VariableDeclarations *>::iterator it = allDeclarations.begin (); it
      != allDeclarations.end (); ++it)
  {
    VariableDeclarations * declarations = *it;

    theDeclarations.insert (declarations->theDeclarations.begin (),
        declarations->theDeclarations.end ());
  }
}
