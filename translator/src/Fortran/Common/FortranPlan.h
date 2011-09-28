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

    virtual SgBasicBlock *
    createConvertPlanFunctionParametersStatements (
        FortranParallelLoop * parallelLoop, SgScopeStatement * subroutineScope,
        SubroutineVariableDeclarations * variableDeclarations);

    virtual SgBasicBlock *
    createConvertPositionInPMapsStatements (FortranParallelLoop * parallelLoop,
        SgScopeStatement * subroutineScope,
        SubroutineVariableDeclarations * variableDeclarations);

    virtual SgBasicBlock *
    createPlanFunctionParametersPreparationStatements (
        FortranParallelLoop * parallelLoop,
        SubroutineVariableDeclarations * variableDeclarations);

    virtual SgFunctionCallExp *
    createPlanFunctionCallExpression (SgScopeStatement * subroutineScope,
        SubroutineVariableDeclarations * variableDeclarations);
};

#endif
