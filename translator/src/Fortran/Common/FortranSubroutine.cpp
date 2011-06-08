#include <FortranSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

FortranSubroutine::FortranSubroutine ()
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
