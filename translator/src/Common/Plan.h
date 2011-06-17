/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class contains functions which must be implemented
 * by any subroutine which includes a plan function.
 *
 * All of the virtual functions are pure to create an interface
 * as the generation of statements depends on the language (i.e.
 * C or Fortran).
 */

#ifndef PLAN_H
#define PLAN_H

#include <rose.h>
#include <VariableDeclarations.h>
#include <ParallelLoop.h>

class Plan
{
  protected:

    /*
     * ======================================================
     * The plan function has a number of arguments which
     * must be initialised before obtaining the plan. This
     * function creates those statements and attaches them
     * to the supplied scope
     * ======================================================
     */
    virtual void
    createPlanFunctionParametersPreparationStatements (
        ParallelLoop * parallelLoop, SgScopeStatement * scope,
        VariableDeclarations * variableDeclarations) = 0;

    /*
     * ======================================================
     * Create the statement which calls the plan function and
     * attaches it to the supplied scope
     * ======================================================
     */
    virtual void
    createPlanFunctionCallStatement (SgScopeStatement * scope,
        VariableDeclarations * variableDeclarations) = 0;

    /*
     * ======================================================
     * The plan function returns a number of fields dictating
     * how to execute over a particular set. This function
     * creates statements to execute that plan and attaches them
     * to the supplied scope
     * ======================================================
     */
    virtual void
        createPlanFunctionExecutionStatements (ParallelLoop * parallelLoop,
            SgScopeStatement * scope,
            VariableDeclarations * variableDeclarations) = 0;
};

#endif
