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

SgProcedureHeaderStatement *
Subroutine::getSubroutineHeaderStatement ()
{
  return subroutineHeaderStatement;
}

SgVariableDeclaration *
Subroutine::getFormalParameterDeclaration (std::string const & variableName)
{
  return formalParameterDeclarations[variableName];
}

SgVariableDeclaration *
Subroutine::getLocalVariableDeclaration (std::string const & variableName)
{
  return localVariableDeclarations[variableName];
}
