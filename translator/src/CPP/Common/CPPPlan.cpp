#include <CPPPlan.h>

SgBasicBlock *
CPPPlan::createPlanFunctionParametersPreparationStatements ()
{
  return NULL;
}

SgFunctionCallExp *
CPPPlan::createPlanFunctionCallExpression ()
{
  return NULL;
}

CPPPlan::CPPPlan (SgScopeStatement * subroutineScope,
    ParallelLoop * parallelLoop,
    ScopedVariableDeclarations * variableDeclarations):
    Plan(subroutineScope, parallelLoop, variableDeclarations)
{
}
