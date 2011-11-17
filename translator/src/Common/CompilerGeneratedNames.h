#pragma once
#ifndef COMPILER_GENERATED_NAMES_H
#define COMPILER_GENERATED_NAMES_H

#include <string>

class SgType;
class SgStatement;
class SgExpression;
class SgScopeStatement;
class SgFunctionCallExp;

namespace ReductionVariableNames
{
  /*
   * ======================================================
   * Variable names used in reductions
   * ======================================================
   */

  std::string const reductionInput = "inputValue";
  std::string const reductionResult = "reductionResult";
  std::string const reductionOperation = "reductionOperation";
  std::string const reductionBytes = "reductionBytes";
  std::string const reductionSharedMemorySize = "reductionSharedMemorySize";

  std::string const
  getReductionArrayHostName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getReductionArrayDeviceName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getReductionCardinalityName (unsigned int OP_DAT_ArgumentGroup);
}

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
  std::string const nelems2 = "nelems2";
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
  getActualPlanVariableName (const std::string suffix = std::string ());

  /*
   * ======================================================
   * Get the variable name for the plan return value with this
   * suffix
   * ======================================================
   */

  std::string const
  getPlanReturnVariableName (const std::string suffix = std::string ());

  /*
   * ======================================================
   * Name of the variable which represents the size of a
   * local to global renumbering (i.e. from local memory
   * into global device memory in the CUDA architecture)
   * argument used for an indirect OP_DAT.
   *
   * These are the 'ind_maps' variable sizes in the plan
   * function according to Mike Giles documentation
   * ======================================================
   */
  std::string const
  getLocalToGlobalMappingName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getLocalToGlobalMappingSizeName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Name of the variable which represents the size of a
   * global to local renumbering (i.e. from global device memory
   * to local memory in the CUDA architecture)
   * argument used for an indirect OP_DAT.
   *
   * These are the 'maps' variable sizes in the plan function
   * according to Mike Giles documentation
   * ======================================================
   */
  std::string const
  getGlobalToLocalMappingName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getGlobalToLocalMappingSizeName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getIndirectionArgumentSizeName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getIndirectionMapName (unsigned int OP_DAT_ArgumentGroup);
}

namespace LoopVariableNames
{
  /*
   * ======================================================
   * Get a for loop counter variable name with this integer
   * suffix
   * ======================================================
   */

  std::string const
  getIterationCounterVariableName (unsigned int n);

  /*
   * ======================================================
   * Get an upper bound variable name with this integer
   * suffix
   * ======================================================
   */

  std::string const
  getUpperBoundVariableName (unsigned int n);
}

namespace OP2VariableNames
{
  std::string const localOffset = "localOffset";
  std::string const moduloResult = "moduloResult";
  std::string const nbytes = "nbytes";
  std::string const opDatCardinalities = "opDatCardinalities";
  std::string const opDatDimensions = "opDatDimensions";
  std::string const partitionSize = "partitionSize";
  std::string const setSize = "setSize";
  std::string const sharedMemoryOffset = "sharedMemoryOffset";
  std::string const sharedMemoryBytes = "sharedMemoryBytes";
  std::string const threadID = "threadID";
  std::string const threadItems = "threadItems";
  std::string const warpSize = "warpSize";
  std::string const warpMemorySize = "warpMemorySize";

  /*
   * ======================================================
   * Returns the name of the formal parameter which models the
   * name of the user subroutine
   * ======================================================
   */
  std::string const
  getUserSubroutineName ();

  /*
   * ======================================================
   * Returns the name of the formal parameter which models the
   * OP_SET
   * ======================================================
   */
  std::string const
  getOpSetName ();

  /*
   * ======================================================
   * Returns the name of the variable for the type OP_SET_CORE
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpSetCoreName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of an OP_DAT variable in this OP_DAT
   * argument group
   * ======================================================
   */
  std::string const
  getOpDatName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of an OP_DAT variable in this OP_DAT
   * argument group on the host
   * ======================================================
   */
  std::string const
  getOpDatHostName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of a local OP_DAT variable in this
   * OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatLocalName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of a global OP_DAT variable in this
   * OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatGlobalName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the variable modelling the size of
   * an OP_DAT in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatCardinalityName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the OP_DAT device variable
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatDeviceName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the OP_DAT core variable
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatCoreName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the OP_DAT dimension variable
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatDimensionName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the indirection variable in this
   * OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpIndirectionName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the mapping variable in this OP_DAT
   * argument group
   * ======================================================
   */
  std::string const
  getOpMapName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the access variable in this OP_DAT
   * argument group
   * ======================================================
   */
  std::string const
  getOpAccessName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the C to Fortran variable
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getCToFortranVariableName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getNumberOfBytesVariableName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getRoundUpVariableName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getIncrementAccessMapName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getIndirectionCUDASharedMemoryName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getCUDASharedMemoryDeclarationName (SgType * type, unsigned int size);

  std::string const
  getCUDAVolatileSharedMemoryDeclarationName (SgType * type, unsigned int size);

  std::string const
  getCUDASharedMemoryOffsetDeclarationName (SgType * type, unsigned int size);
}

#endif
