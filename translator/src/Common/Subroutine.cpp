#include <boost/lexical_cast.hpp>
#include <CommonNamespaces.h>
#include <Subroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

std::string
Subroutine::get_OP_DAT_FormalParameterName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

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
