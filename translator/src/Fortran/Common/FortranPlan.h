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
        SubroutineVariableDeclarations * variableDeclarations,
        FortranParallelLoop * parallelLoop, SgScopeStatement * subroutineScope);

    virtual SgBasicBlock *
    createConvertPositionInPMapsStatements (
        SubroutineVariableDeclarations * variableDeclarations,
        FortranParallelLoop * parallelLoop, SgScopeStatement * subroutineScope);

    virtual SgBasicBlock *
    createPlanFunctionParametersPreparationStatements (
        SubroutineVariableDeclarations * variableDeclarations,
        FortranParallelLoop * parallelLoop);

    virtual SgBasicBlock *
    createPlanFunctionCallStatement (
        SubroutineVariableDeclarations * variableDeclarations,
        SgScopeStatement * subroutineScope);
};

#endif
