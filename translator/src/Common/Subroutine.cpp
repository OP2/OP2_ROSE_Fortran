#include <Subroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

Subroutine::Subroutine (std::string const & subroutineName)
{
  using SageBuilder::buildFunctionParameterList;

  this->subroutineName = subroutineName;

  formalParameters = buildFunctionParameterList ();
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

std::string const &
Subroutine::getSubroutineName () const
{
  return subroutineName;
}

SgVariableDeclaration *
Subroutine::getVariableDeclaration (std::string const & variableName)
{
  return variableDeclarations[variableName];
}
