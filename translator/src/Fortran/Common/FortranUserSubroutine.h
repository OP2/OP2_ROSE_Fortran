#ifndef FORTRAN_USER_SUBROUTINE_H
#define FORTRAN_USER_SUBROUTINE_H

#include <UserSubroutine.h>

class FortranParallelLoop;
class FortranProgramDeclarationsAndDefinitions;

class FortranUserSubroutine: public UserSubroutine <SgProcedureHeaderStatement,
    FortranProgramDeclarationsAndDefinitions>
{
  public:

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

    FortranUserSubroutine (SgScopeStatement * moduleScope,
        FortranParallelLoop * parallelLoop,
        FortranProgramDeclarationsAndDefinitions * declarations);
};

#endif
