#pragma once
#ifndef CPPPLAN_H
#define CPPPLAN_H

#include <Plan.h>

class CPPPlan: public Plan
{
  public:

    virtual SgBasicBlock *
    createPlanFunctionParametersPreparationStatements ();

    virtual SgFunctionCallExp *
    createPlanFunctionCallExpression ();

    CPPPlan (SgScopeStatement * subroutineScope, ParallelLoop * parallelLoop,
        ScopedVariableDeclarations * variableDeclarations);
};

#endif
