#pragma once
#ifndef PLAN_FUNCTION_H
#define PLAN_FUNCTION_H

#include <string>

namespace PlanFunction
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
  std::string const blockOffset = "blockOffset";
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

#endif
