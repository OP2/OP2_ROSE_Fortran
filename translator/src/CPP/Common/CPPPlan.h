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
        SubroutineVariableDeclarations * variableDeclarations);

    virtual SgFunctionCallExp *
    createPlanFunctionCallExpression (SgScopeStatement * subroutineScope,
        SubroutineVariableDeclarations * variableDeclarations);
};

#endif
