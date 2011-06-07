#include <Subroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

Subroutine::Subroutine (std::string const & subroutineName)
{
  this->subroutineName = subroutineName;
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
