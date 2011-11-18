#pragma once
#ifndef PLAN_FUNCTION_NAMES_H
#define PLAN_FUNCTION_NAMES_H

#include <string>

namespace PlanFunctionVariableNames
{
  /*
   * ======================================================
   * Variable names used in the plan function
   * ======================================================
   */

  std::string const accesses = "accesses";
  std::string const actualPlan = "actualPlan";
  std::string const args = "args";
  std::string const argsNumber = "argsNumber";
  std::string const blkmap = "blkmap";
  std::string const blockID = "blockID";
  std::string const blockOffset = "blockOffset";
  std::string const colour1 = "colour1";
  std::string const colour2 = "colour2";
  std::string const cplan = "cplan";
  std::string const idxs = "idxs";
  std::string const inds = "inds";
  std::string const indsNumber = "indsNumber";
  std::string const ind_maps = "ind_maps";
  std::string const ind_offs = "ind_offs";
  std::string const ind_sizes = "ind_sizes";
  std::string const loc_maps = "loc_maps";
  std::string const maps = "maps";
  std::string const nblocks = "nblocks";
  std::string const ncolblk = "ncolblk";
  std::string const ncolors = "ncolors";
  std::string const nelems = "nelems";
  std::string const nindirect = "nindirect";
  std::string const nshared = "nshared";
  std::string const nthrcol = "nthrcol";
  std::string const numberOfColours = "numOfColours";
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

  /*
   * ======================================================
   * Get the variable name for an actual plan with this
   * suffix
   * ======================================================
   */

  std::string const
  getActualPlanVariableName (std::string const & suffix = std::string ());

  /*
   * ======================================================
   * Get the variable name for the plan return value with this
   * suffix
   * ======================================================
   */

  std::string const
  getPlanReturnVariableName (std::string const & suffix = std::string ());

  std::string const
  getNumberOfThreadColoursPerBlockArrayName (std::string const & suffix =
      std::string ());

  std::string const
  getNumberOfThreadColoursPerBlockSizeName (std::string const & suffix =
      std::string ());

  std::string const
  getThreadColourArrayName (std::string const & suffix = std::string ());

  std::string const
  getThreadColourSizeName (std::string const & suffix = std::string ());

  std::string const
  getOffsetIntoBlockArrayName (std::string const & suffix = std::string ());

  std::string const
  getOffsetIntoBlockSizeName (std::string const & suffix = std::string ());

  std::string const
  getIndirectOpDatsArrayName (std::string const & suffix = std::string ());

  /*
   * ======================================================
   * Following are variable names used to map data from
   * local memory into global memory.
   *
   * These are the 'ind_maps' symbols in Mike Giles's
   * implementation
   * ======================================================
   */

  std::string const
  getIndirectOpDatsLocalToGlobalMappingName (std::string const & suffix =
      std::string ());

  std::string const
  getLocalToGlobalMappingName (unsigned int OP_DAT_ArgumentGroup,
      std::string const & suffix = std::string ());

  std::string const
  getLocalToGlobalMappingSizeName (unsigned int OP_DAT_ArgumentGroup,
      std::string const & suffix = std::string ());

  std::string const
      getIndirectOpDatsOffsetArrayName (std::string const & suffix =
          std::string ());

  std::string const
  getIndirectOpDatsOffsetSizeName (std::string const & suffix = std::string ());

  std::string const
  getIndirectOpDatsNumberOfElementsArrayName (std::string const & suffix =
      std::string ());

  std::string const
  getIndirectOpDatsNumberOfElementsSizeName (std::string const & suffix =
      std::string ());

  std::string const
  getOpDatsGlobalToLocalMappingName (std::string const & suffix =
      std::string ());

  /*
   * ======================================================
   * Following are variable names used to map data from
   * global memory into local memory.
   *
   * These are the 'map' symbols in Mike Giles's
   * implementation
   * ======================================================
   */

  std::string const
  getGlobalToLocalMappingName (unsigned int OP_DAT_ArgumentGroup,
      std::string const & suffix = std::string ());

  std::string const
  getGlobalToLocalMappingSizeName (unsigned int OP_DAT_ArgumentGroup,
      std::string const & suffix = std::string ());

  std::string const
  getNumberOfSetElementsPerBlockArrayName (std::string const & suffix =
      std::string ());

  std::string const
  getNumberOfSetElementsPerBlockSizeName (std::string const & suffix =
      std::string ());

  std::string const
  getNumberOfBlockColoursName (std::string const & suffix = std::string ());

  std::string const
  getColourToNumberOfBlocksArrayName (std::string const & suffix =
      std::string ());

  std::string const
  getColourToBlockArrayName (std::string const & suffix = std::string ());

  std::string const
  getColourToBlockSizeName (std::string const & suffix = std::string ());

  std::string const
  getIndirectionArgumentSizeName (unsigned int OP_DAT_ArgumentGroup,
      std::string const & suffix = std::string ());

  std::string const
  getIndirectionMapName (unsigned int OP_DAT_ArgumentGroup,
      std::string const & suffix = std::string ());
}

#endif
