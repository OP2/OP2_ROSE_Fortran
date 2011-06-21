/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class contains functions which must be implemented
 * by any subroutine which includes a plan function.
 *
 * All of the virtual functions are pure to create an interface
 * as the generation of statements depends on the language (i.e.
 * C or Fortran).
 */

#ifndef PLAN_H
#define PLAN_H

#include <rose.h>
#include <VariableDeclarations.h>
#include <ParallelLoop.h>

namespace PlanFunction
{
  /*
   * ======================================================
   * Following are variable names used in the plan function
   * ======================================================
   */
  namespace Fortran
  {
    std::string const blockOffset = "blockOffset";
    std::string const pblkMap = "pblkMap";
    std::string const pindMaps = "pindMaps";
    std::string const pindOffs = "pindOffs";
    std::string const pindSizes = "pindSizes";
    std::string const poffset = "poffset";
    std::string const pmaps = "pmaps";
    std::string const pnelems = "pnelems";
    std::string const pnindirect = "pnindirect";
    std::string const pnthrcol = "pnthrcol";
    std::string const pthrcol = "pthrcol";
    std::string const pblkMapSize = "pblkMapSize";
    std::string const pindMapsSize = "pindMapsSize";
    std::string const pindOffsSize = "pindOffsSize";
    std::string const pindSizesSize = "pindSizesSize";
    std::string const poffsetSize = "poffsetSize";
    std::string const pnelemsSize = "pnelemsSize";
    std::string const pnthrcolSize = "pnthrcolSize";
    std::string const pthrcolSize = "pthrcolSize";
  }
}

class Plan
{
  protected:

    /*
     * ======================================================
     * The plan function has a number of arguments which
     * must be initialised before obtaining the plan. This
     * function creates those statements and attaches them
     * to the supplied scope
     * ======================================================
     */
    virtual void
    createPlanFunctionParametersPreparationStatements (
        ParallelLoop * parallelLoop, SgScopeStatement * scope,
        VariableDeclarations * variableDeclarations) = 0;

    /*
     * ======================================================
     * Create the statement which calls the plan function and
     * attaches it to the supplied scope
     * ======================================================
     */
    virtual void
    createPlanFunctionCallStatement (SgScopeStatement * scope,
        VariableDeclarations * variableDeclarations) = 0;

    /*
     * ======================================================
     * The plan function returns a number of fields dictating
     * how to execute over a particular set. This function
     * creates statements to execute that plan and attaches them
     * to the supplied scope
     * ======================================================
     */
    virtual void
        createPlanFunctionExecutionStatements (ParallelLoop * parallelLoop,
            SgScopeStatement * scope,
            VariableDeclarations * variableDeclarations) = 0;
};

#endif
