#pragma once
#ifndef FORTRAN_PLAN_H
#define FORTRAN_PLAN_H

#include <Plan.h>

class FortranPlan: public Plan
{
  public:

    SgBasicBlock *
    createConvertPositionInPMapsStatements ();

    virtual SgBasicBlock *
    createConvertPlanFunctionParametersStatements ();

    virtual SgBasicBlock *
    createPlanFunctionParametersPreparationStatements ();

    virtual SgFunctionCallExp *
    createPlanFunctionCallExpression ();

    FortranPlan (SgScopeStatement * subroutineScope,
        ParallelLoop * parallelLoop,
        ScopedVariableDeclarations * variableDeclarations);
};

#endif
