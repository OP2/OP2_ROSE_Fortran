/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class contains additional functions required in
 * Fortran when a plan function is involved
 */

#pragma once
#ifndef FORTRAN_PLAN_H
#define FORTRAN_PLAN_H

#include <Plan.h>

class ParallelLoop;

class FortranPlan: public Plan
{
  public:

    virtual SgBasicBlock *
    createConvertPlanFunctionParametersStatements (ParallelLoop * parallelLoop,
        SgScopeStatement * subroutineScope,
        ScopedVariableDeclarations * variableDeclarations);

    virtual SgBasicBlock *
    createConvertPositionInPMapsStatements (ParallelLoop * parallelLoop,
        SgScopeStatement * subroutineScope,
        ScopedVariableDeclarations * variableDeclarations);

    virtual SgBasicBlock *
    createPlanFunctionParametersPreparationStatements (
        ParallelLoop * parallelLoop,
        ScopedVariableDeclarations * variableDeclarations);

    virtual SgFunctionCallExp *
    createPlanFunctionCallExpression (SgScopeStatement * subroutineScope,
        ScopedVariableDeclarations * variableDeclarations);
};

#endif
