#ifndef FORTRAN_USER_SUBROUTINE_H
#define FORTRAN_USER_SUBROUTINE_H

#include <UserSubroutine.h>

class FortranParallelLoop;
class FortranProgramDeclarationsAndDefinitions;

class FortranUserSubroutine: public UserSubroutine <SgProcedureHeaderStatement,
    FortranProgramDeclarationsAndDefinitions>
{
  protected:

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranUserSubroutine (SgScopeStatement * moduleScope,
        FortranParallelLoop * parallelLoop,
        FortranProgramDeclarationsAndDefinitions * declarations);
};

#endif
