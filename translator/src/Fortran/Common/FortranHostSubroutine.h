#ifndef FORTRAN_HOST_SUBROUTINE_H
#define FORTRAN_HOST_SUBROUTINE_H

#include <HostSubroutine.h>
#include <FortranSubroutine.h>
#include <ParallelLoop.h>
#include <Plan.h>

class FortranHostSubroutine: public FortranSubroutine,
    public HostSubroutine,
    public Plan
{
  protected:

    /*
     * ======================================================
     * Returns a statement which represents a C-to-Fortran
     * pointer conversion
     * ======================================================
     */
    SgStatement *
    createCToFortranPointerCall (SgExpression * parameter1,
        SgExpression * parameter2, SgExpression * parameter3 = NULL);

    virtual void
    createStatementsToPreparePlanFunctionParameters (
        std::vector <SgStatement *> & statements);

    virtual SgStatement *
    createStatementToCallPlanFunction ();

    virtual void
    createStatementsToExecutePlanFunction (
        std::vector <SgStatement *> & statements);

    virtual void
    createFormalParameterDeclarations ();

    FortranHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
