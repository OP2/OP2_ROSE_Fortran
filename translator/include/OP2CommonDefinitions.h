/*
 * Written by Carlo Bertolli and Adam Betts
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
  std::string const OP_PAR_LOOP_PREFIX = "op_par_loop";
  std::string const OP_DECL_SET_PREFIX = "op_decl_set";
  std::string const OP_DECL_MAP_PREFIX = "op_decl_map";
  std::string const OP_DECL_DAT_PREFIX = "op_decl_dat";

  /*
   * Positions of arguments inside OP_PAR_LOOP
   * The following style is assumed:
   * OP_PAR_LOOP_<n> (kernel, iteration_set,
   *                  arg0, idx0, map0, acc0,
   *                  ...
   *                  argN-1, idxN-1, mapN-1, accN-1)
  */
  unsigned int const NUMBER_OF_OP_DAT_ARGUMENTS = 4;
  unsigned int const BASE_OP_DAT_ARG_INDEX = 2;

  /*
   * Positions of arguments inside OP_DECL_SET
   * The following style is assumed:
   * OP_DECL_SET (setCardinality, OP_SET_REF)
   */
  unsigned int const OP_DECL_SET_VAR_REF = 1;

  /*
   * Positions of arguments inside OP_DECL_MAP
   * The following style is assumed:
   * OP_DECL_MAP (fromSet, toSet, cardinality, inputData, op_map_ref)
   */
  unsigned int const OP_DECL_MAP_VAR_REF = 4;

  /*
   * Positions of arguments inside OP_DECL_DAT
   * The following style is assumed:
   * OP_DECL_DAT (set, cardinality, inputData, op_dat_ref)
   */
  unsigned int const OP_DECL_DAT_DIMENSION_VAL = 1;
  unsigned int const OP_DECL_DAT_INPUT_REF = 2;
  unsigned int const OP_DECL_DAT_VAR_REF = 3;
}

#endif
