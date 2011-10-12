#pragma once
#ifndef PLAN_H
#define PLAN_H

#include <string>

class SgBasicBlock;
class SgFunctionCallExp;
class SgScopeStatement;
class ParallelLoop;
class ScopedVariableDeclarations;

class Plan
{
  protected:

    SgScopeStatement * subroutineScope;

    ParallelLoop * parallelLoop;

    ScopedVariableDeclarations * variableDeclarations;

  protected:

    /*
     * ======================================================
     * The plan function has a number of arguments which
     * must be initialised before obtaining the plan. This
     * function creates those statements
     * ======================================================
     */
    virtual SgBasicBlock *
    createPlanFunctionParametersPreparationStatements () = 0;

    /*
     * ======================================================
     * Create the expression representing the call to the plan
     * function. The scope of the enclosing subroutine is also
     * needed to build the function call correctly
     * ======================================================
     */
    virtual SgFunctionCallExp *
    createPlanFunctionCallExpression () = 0;

    Plan (SgScopeStatement * subroutineScope, ParallelLoop * parallelLoop,
        ScopedVariableDeclarations * variableDeclarations) :
      subroutineScope (subroutineScope), parallelLoop (parallelLoop),
          variableDeclarations (variableDeclarations)
    {
    }
};

#endif
