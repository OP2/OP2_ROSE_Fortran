#pragma once
#ifndef CPPPLAN_H
#define CPPPLAN_H

#include <Plan.h>

class CPPPlan: public Plan
{
  protected:

    virtual SgBasicBlock *
    createPlanFunctionParametersPreparationStatements (
        ParallelLoop * parallelLoop,
        ScopedVariableDeclarations * variableDeclarations);

    virtual SgFunctionCallExp *
    createPlanFunctionCallExpression (SgScopeStatement * subroutineScope,
        ScopedVariableDeclarations * variableDeclarations);
};

#endif
