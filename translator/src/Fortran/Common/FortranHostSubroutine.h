#ifndef FORTRAN_HOST_SUBROUTINE_H
#define FORTRAN_HOST_SUBROUTINE_H

#include <FortranSubroutine.h>
#include <ParallelLoop.h>

class FortranHostSubroutine: public FortranSubroutine
{
  protected:

    std::string userSubroutineName;

    std::string kernelSubroutineName;

    ParallelLoop * parallelLoop;

  protected:

    /*
     * ======================================================
     * Creates the function call which converts a C execution
     * plan onto the Fortran side
     * ======================================================
     */
    void
    createPlanFunctionCallStatement ();

    /*
     * ======================================================
     * Returns the sequence of statements which initialise
     * variables used in the execution plan
     * ======================================================
     */
    void
    createInitialiseExecutionPlanStatements (
        std::vector <SgStatement *> & statements);

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
    createlocalVariableDeclarations ();

    FortranHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop);
};

#endif
