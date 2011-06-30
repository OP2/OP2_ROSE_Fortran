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
    std::string const accesses = "accesses";
    std::string const actualPlan = "actualPlan";
    std::string const args = "args";
    std::string const argsNumber = "argsNumber";
    std::string const blkmap = "blkmap";
    std::string const blockOffset = "blockOffset";
    std::string const cplan = "cplan";
    std::string const idxs = "idxs";
    std::string const inds = "inds";
    std::string const indsNumber = "indsNumber";
    std::string const ind_maps = "ind_maps";
    std::string const ind_offs = "ind_offs";
    std::string const ind_sizes = "ind_sizes";
    std::string const maps = "maps";
    std::string const nblocks = "nblocks";
    std::string const ncolblk = "ncolblk";
    std::string const ncolors = "ncolors";
    std::string const nelems = "nelems";
    std::string const nindirect = "nindirect";
    std::string const nshared = "nshared";
    std::string const nthrcol = "nthrcol";
    std::string const offset = "offset";
    std::string const pblkMap = "pblkMap";
    std::string const pblkMapSize = "pblkMapSize";
    std::string const pindMaps = "pindMaps";
    std::string const pindMapsSize = "pindMapsSize";
    std::string const pindOffs = "pindOffs";
    std::string const pindOffsSize = "pindOffsSize";
    std::string const pindSizes = "pindSizes";
    std::string const pindSizesSize = "pindSizesSize";
    std::string const planRet = "planRet";
    std::string const pmaps = "pmaps";
    std::string const pnelems = "pnelems";
    std::string const pnelemsSize = "pnelemsSize";
    std::string const pnindirect = "pnindirect";
    std::string const pnthrcol = "pnthrcol";
    std::string const pnthrcolSize = "pnthrcolSize";
    std::string const poffset = "poffset";
    std::string const poffsetSize = "poffsetSize";
    std::string const pthrcol = "pthrcol";
    std::string const pthrcolSize = "pthrcolSize";
    std::string const thrcol = "thrcol";
  }
}

template <typename TParallelLoop>
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
          VariableDeclarations * variableDeclarations,
          TParallelLoop * parallelLoop, SgScopeStatement * scope) = 0;

      /*
       * ======================================================
       * Create the statement which calls the plan function and
       * attaches it to the supplied scope. The scope of the
       * enclosing subroutine is also needed to build the
       * function call correctly
       * ======================================================
       */
      virtual void
          createPlanFunctionCallStatement (
              VariableDeclarations * variableDeclarations,
              SgScopeStatement * subroutineScope,
              SgScopeStatement * statementScope) = 0;

      /*
       * ======================================================
       * The plan function returns a number of fields dictating
       * how to execute over a particular set. This function
       * creates statements to execute that plan and attaches them
       * to the supplied scope
       * ======================================================
       */
      virtual void
      createPlanFunctionExecutionStatements () = 0;
  };

#endif
