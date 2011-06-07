#include <FortranSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */
FortranSubroutine::FortranSubroutine (std::string const & subroutineName) :
  Subroutine (subroutineName)
{

}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgProcedureHeaderStatement *
FortranSubroutine::getProcedureHeaderStatement ()
{
  return subroutineHeaderStatement;
}
