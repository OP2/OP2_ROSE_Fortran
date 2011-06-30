/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class contains additional functions required in
 * Fortran when a plan function is involved
 */

#ifndef FORTRAN_PLAN_H
#define FORTRAN_PLAN_H

#include <Plan.h>
#include <FortranParallelLoop.h>

class FortranPlan: public Plan <FortranParallelLoop>
{
  protected:

    virtual void
    createConvertPlanFunctionParametersStatements (
        VariableDeclarations * variableDeclarations,
        FortranParallelLoop * parallelLoop, SgScopeStatement * subroutineScope,
        SgScopeStatement * statementScope);

    virtual void
    createConvertPositionInPMapsStatements (
        VariableDeclarations * variableDeclarations,
        FortranParallelLoop * parallelLoop, SgScopeStatement * subroutineScope,
        SgScopeStatement * statementScope);

    virtual void
    createPlanFunctionParametersPreparationStatements (
        VariableDeclarations * variableDeclarations,
        FortranParallelLoop * parallelLoop, SgScopeStatement * scope);

    virtual void
    createPlanFunctionCallStatement (
        VariableDeclarations * variableDeclarations,
        SgScopeStatement * subroutineScope, SgScopeStatement * statementScope);
};

#endif
