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

#ifndef PLAN_H
#define PLAN_H

#include <rose.h>
#include <SubroutineVariableDeclarations.h>
#include <ParallelLoop.h>

namespace PlanFunction
{
  /*
   * ======================================================
   * Following are variable names used in the plan function
   * ======================================================
   */

  namespace CPP
  {
    std::string const accesses = "accesses";
    std::string const actualPlan = "actualPlan";
    std::string const args = "args";
    std::string const argsNumber = "nargs";
    std::string const blkmap = "blkmap";
    std::string const blockOffset = "blockOffset";
    std::string const cplan = "cplan";
    std::string const idxs = "idxs";
    std::string const inds = "inds";
    std::string const indsNumber = "ninds";
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
    std::string const pblkMap = "blkmap";
    std::string const partSize = "part_size";
    std::string const pblkMapSize = "pblkMapSize";
    std::string const pindMaps = "pindMaps";
    std::string const pindMapsSize = "pindMapsSize";
    std::string const pindOffs = "ind_arg_offs";
    std::string const pindOffsSize = "pindOffsSize";
    std::string const pindSizes = "ind_arg_sizes";
    std::string const pindSizesSize = "pindSizesSize";
    std::string const planRet = "planRet";
    std::string const pmaps = "pmaps";
    std::string const pnelems = "nelems";
    std::string const pnelemsSize = "pnelemsSize";
    std::string const pnindirect = "pnindirect";
    std::string const pnthrcol = "ncolors";
    std::string const pnthrcolSize = "pnthrcolSize";
    std::string const poffset = "offset";
    std::string const poffsetSize = "poffsetSize";
    std::string const pthrcol = "colors";
    std::string const pthrcolSize = "pthrcolSize";
    std::string const thrcol = "thrcol";
    std::string const plan = "Plan";
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
       * function creates those statements
       * ======================================================
       */
      virtual SgBasicBlock *
      createPlanFunctionParametersPreparationStatements (
          TParallelLoop * parallelLoop,
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
