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

    virtual void
    createConvertPlanFunctionParametersStatements (
        VariableDeclarations * variableDeclarations,
        ParallelLoop * parallelLoop, SgScopeStatement * subroutineScope,
        SgScopeStatement * statementScope);

    virtual void
    createConvertPositionInPMapsStatements (
        VariableDeclarations * variableDeclarations,
        ParallelLoop * parallelLoop, SgScopeStatement * subroutineScope,
        SgScopeStatement * statementScope);

    virtual void
    createPlanFunctionParametersPreparationStatements (
        VariableDeclarations * variableDeclarations,
        ParallelLoop * parallelLoop, SgScopeStatement * scope);

    virtual void
    createPlanFunctionCallStatement (
        VariableDeclarations * variableDeclarations,
        SgScopeStatement * subroutineScope, SgScopeStatement * statementScope);
};

#endif
