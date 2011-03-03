/*
 * Written by Adam Betts and Carlo Bertolli
 *  
 * Includes macros of all common configuration parameters of the OP2 API
 * as well as a namespace for the various names that OP2 employs
 */

#ifndef OP2_COMMON_DEFINITIONS_H
#define OP2_COMMON_DEFINITIONS_H

namespace OP2
{
  /*
   * Declaration of names specific to OP2
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
   * Positions of arguments inside OP_PAR_LOOP
   * The following style is assumed:
   * OP_PAR_LOOP_<N> (kernelName, OP_SET,
   *                  OP_DAT,     int,     OP_MAP,     OP_ACCESS,
   *                  ...
   *                  OP_DAT_N-1, int_N-1, OP_MAP_N-1, OP_ACCESS_N-1)
   */
  unsigned int const NUMBER_OF_ARGUMENTS_PER_OP_DAT = 4; // For every OP_DAT, there is an int, OP_MAP, and an OP_ACCESS
  unsigned int const NUMBER_OF_NON_OP_DAT_ARGUMENTS = 2; // 2 because of 'kernelName' and 'OP_SET' arguments

  /*
   * Positions of arguments inside OP_DECL_SET
   * The following style is assumed:
   * OP_DECL_SET (setCardinality, OP_SET)
   */
  unsigned int const INDEX_OF_OP_SET_IN_OP_DECL_SET_PARAMETER_LIST = 1;

  /*
   * Positions of arguments inside OP_DECL_MAP
   * The following style is assumed:
   * OP_DECL_MAP (OP_SET_FROM, OP_SET_TO, cardinality, inputData, OP_MAP)
   */
  unsigned int const INDEX_OF_OP_MAP_IN_OP_DECL_MAP_PARAMETER_LIST = 4;

  /*
   * Positions of arguments inside OP_DECL_DAT
   * The following style is assumed:
   * OP_DECL_DAT (OP_SET, cardinality, inputData, OP_DAT)
   */
  unsigned int const INDEX_OF_DIMENSION_IN_OP_DECL_DAT_PARAMETER_LIST = 1;
  unsigned int const INDEX_OF_DATA_TYPE_IN_OP_DECL_DAT_PARAMETER_LIST = 2;
  unsigned int const INDEX_OF_OP_DAT_IN_OP_DECL_DAT_PARAMETER_LIST = 3;

  /*
   * Positions of arguments inside OP_DECL_GBL
   * The following style is assumed:
   * OP_DECL_GBL (OP_SET, cardinality, OP_DAT)
   */
  unsigned int const INDEX_OF_OP_DAT_IN_OP_DECL_GBL_PARAMETER_LIST = 2;
}

#endif
