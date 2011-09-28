/*
 * CPPPlan.h
 *
 *  Created on: 28 Jul 2011
 *      Author: Tiziano Santoro
 */

#ifndef CPPPLAN_H
#define CPPPLAN_H

#include <Plan.h>
#include <CPPParallelLoop.h>

class CPPPlan: public Plan <CPPParallelLoop>
{
  protected:

    virtual SgBasicBlock *
    createPlanFunctionParametersPreparationStatements (
        CPPParallelLoop * parallelLoop,
        SubroutineVariableDeclarations * variableDeclarations);

    virtual SgFunctionCallExp *
    createPlanFunctionCallExpression (SgScopeStatement * subroutineScope,
        SubroutineVariableDeclarations * variableDeclarations);
};

#endif /* CPPPLAN_H_ */
