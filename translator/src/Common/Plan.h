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

class Plan
{
  protected:

    /*
     * ======================================================
     * The plan function has a number of arguments which
     * must be initialised before obtaining the plan. This
     * function creates those statements and puts them into
     * the supplied vector
     * ======================================================
     */
    virtual void
    createStatementsToPreparePlanFunctionParameters (
        std::vector <SgStatement *> & statements) = 0;

    /*
     * ======================================================
     * Create the statement which calls the plan function
     * ======================================================
     */
    virtual SgStatement *
    createStatementToCallPlanFunction () = 0;

    /*
     * ======================================================
     * The plan function returns a number of fields dictating
     * how to execute over a particular set. This function
     * creates statements to execute that plan and puts them
     * into the supplied vector
     * ======================================================
     */
    virtual void
    createStatementsToExecutePlanFunction (
        std::vector <SgStatement *> & statements) = 0;

    /*
     * ======================================================
     * Create the statement which calls the kernel function
     * ======================================================
     */
    virtual SgStatement *
    createStatementToCallKernelFunction () = 0;
};

#endif
