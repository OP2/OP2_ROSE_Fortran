/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class contains additional functions required in
 * Fortran when a plan function is involved
 */

#ifndef FORTRAN_PLAN_H
#define FORTRAN_PLAN_H

#include <Plan.h>

class FortranPlan: public Plan
{
  protected:

    /*
     * ======================================================
     * All the data returned by the plan function is contained
     * in a C interface. This function creates statements to
     * convert that data onto the Fortran side and attaches
     * the statements to the supplied scope
     * ======================================================
     */
    virtual void
    createConvertCPointersStatements () = 0;

    virtual void
    createPlanFunctionParametersPreparationStatements (
        ParallelLoop * parallelLoop, SgScopeStatement * scope,
        VariableDeclarations * variableDeclarations);

    virtual SgStatement *
    createPlanFunctionCallStatement (SgScopeStatement * scope,
        VariableDeclarations * variableDeclarations);

    virtual void
    createPlanFunctionExecutionStatements (ParallelLoop * parallelLoop,
        SgScopeStatement * scope, VariableDeclarations * variableDeclarations);
};

#endif
