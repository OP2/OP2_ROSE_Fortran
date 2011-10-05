/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class contains functions which must be implemented
 * by any subroutine which includes a plan function.
 *
 * All of the virtual functions are pure to create an interface
 * as the generation of statements depends on the language (i.e.
 * C++ or Fortran).
 */

#pragma once
#ifndef PLAN_H
#define PLAN_H

#include <string>

class SgBasicBlock;
class SgFunctionCallExp;
class SgScopeStatement;
class SubroutineVariableDeclarations;
class ParallelLoop;

class Plan
{
  public:

    /*
     * ======================================================
     * The plan function has a number of arguments which
     * must be initialised before obtaining the plan. This
     * function creates those statements
     * ======================================================
     */
    virtual SgBasicBlock *
    createPlanFunctionParametersPreparationStatements (
        ParallelLoop * parallelLoop,
        SubroutineVariableDeclarations * variableDeclarations) = 0;

    /*
     * ======================================================
     * Create the expression representing the call to the plan
     * function. The scope of the enclosing subroutine is also
     * needed to build the function call correctly
     * ======================================================
     */
    virtual SgFunctionCallExp *
    createPlanFunctionCallExpression (SgScopeStatement * subroutineScope,
        SubroutineVariableDeclarations * variableDeclarations) = 0;

    /*
     * ======================================================
     * The plan function returns a number of fields dictating
     * how to execute over a particular set. This function
     * creates statements to execute that plan
     * ======================================================
     */
    virtual void
    createPlanFunctionExecutionStatements () = 0;
};

#endif
