/*
 * Written by Adam Betts and Carlo Bertolli
 *  
 * A namespace for the various names that OP2 employs
 */

#ifndef OP2_COMMON_DEFINITIONS_H
#define OP2_COMMON_DEFINITIONS_H

namespace OP2
{
  /*
   * ======================================================
   * Declaration of names specific to OP2
   * ======================================================
   */

  std::string const OP_SET_NAME = "op_set";
  std::string const OP_DAT_NAME = "op_dat";
  std::string const OP_MAP_NAME = "op_map";
  std::string const OP_ID_NAME = "op_id";
  std::string const OP_GBL_NAME = "op_gbl";
  std::string const OP_READ_NAME = "op_read";
  std::string const OP_WRITE_NAME = "op_write";
  std::string const OP_RW_NAME = "op_rw";
  std::string const OP_INC_NAME = "op_inc";
  std::string const OP_MAX_NAME = "op_max";
  std::string const OP_MIN_NAME = "op_min";
  std::string const OP_PAR_LOOP_PREFIX = "op_par_loop";
  std::string const OP_DECL_SET_PREFIX = "op_decl_set";
  std::string const OP_DECL_MAP_PREFIX = "op_decl_map";
  std::string const OP_DECL_DAT_PREFIX = "op_decl_dat";
  std::string const OP_DECL_GBL_PREFIX = "op_decl_gbl";

  /*
   * ======================================================
   * The following constants assume that each call to an
   * OP_PAR_LOOP in Fortran is constructed as follows:
   *
   * OP_PAR_LOOP_<N>
   *   (nameOfUserSubroutine,
   *    OP_SET,
   *    OP_DAT,   int,   OP_MAP,   OP_ACCESS,
   *    ...
   *    OP_DAT_N, int_N, OP_MAP_N, OP_ACCESS_N)
   * ======================================================
   */

  /*
   * ======================================================
   * Currently, for every OP_DAT, there is an int, an OP_MAP,
   * and an OP_ACCESS
   * ======================================================
   */
  unsigned int const NUMBER_OF_ARGUMENTS_PER_OP_DAT = 4;

  /*
   * ======================================================
   * There are 2 non-OP_DAT arguments, namely
   * 'nameOfUserSubroutine' and 'OP_SET'
   * ======================================================
   */
  unsigned int const NUMBER_OF_NON_OP_DAT_ARGUMENTS = 2;
	
  /*
   * ======================================================
   * Position of first OP_ACCESS (the following ones
	 * are in following + NUMBER_OF_ARGUMENTS_PER_OP_DAT
	 * positions 
   * ======================================================
   */
  unsigned int const POSITION_OF_FIRST_OP_ACCESS = 6;
	
	
}

namespace PlanFunctionVariables
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
}

namespace PlanFunctionSizeVariables
{
  std::string const pblkMapSize = "pblkMapSize";
  std::string const pindMapsSize = "pindMapsSize";
  std::string const pindOffsSize = "pindOffsSize";
  std::string const pindSizesSize = "pindSizesSize";
  std::string const poffsetSize = "poffsetSize";
  std::string const pnelemsSize = "pnelemsSize";
  std::string const pnthrcolSize = "pnthrcolSize";
  std::string const pthrcolSize = "pthrcolSize";
}

namespace LoopVariables
{
  std::string const argsSizes = "argsSizes";
}

namespace kernelDatArgumentsNames
{
	std::string const argNamePrefix = "argument";
	std::string const argNameSizePostfix = "Size";
}

namespace kernelLocalThreadVarsNames
{
	std::string const localVarNamePrefix = "arg";
	std::string const localVarNamePostfix = "_l";
}

namespace reductionSubroutineNames
{
	std::string const reductionSubroutinePostfix = "_reduction";
	std::string const reductionSubroutineTypeInteger = "_integer";
	std::string const reductionSubroutineTypeReal = "_real";
	std::string const iterationVarForReductionName1 = "iterationRed1";
	std::string const iterationVarForReductionName2 = "iterationRed2";
}


namespace FortranVariableDeafultKinds
{
	int const DEFAULT_KIND_INT = 4;
	int const DEFAULT_KIND_REAL = 4;
}

#endif
