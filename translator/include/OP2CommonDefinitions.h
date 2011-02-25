/*
 * Written by Carlo Bertolli and Adam Betts
 *  
 * Includes macros of all common configuration parameters of the OP2 API
 * as well as a namespace for the various names OP2 employs
 */

#ifndef OP2_COMMON_DEFINITIONS_H
#define OP2_COMMON_DEFINITIONS_H

/*
 * Declaration of names specific to OP2
 */
namespace OP2
{
  using namespace std;

  string const OP_SET_NAME = "op_set";
  string const OP_DAT_NAME = "op_dat";
  string const OP_MAP_NAME = "op_map";
  string const OP_PAR_LOOP_PREFIX = "op_par_loop";
  string const OP_DECL_SET_PREFIX = "op_decl_set";
  string const OP_DECL_MAP_PREFIX = "op_decl_map";
  string const OP_DECL_DAT_PREFIX = "op_decl_dat";
}

// Macros for positions inside the op_par_loop arguments
// we assume the following style:
// op_par_loop_* (kernel, iteration_set,
//		  arg0, idx0, map0, acc0,
//		  ...
//		  argN-1, idxN-1, mapN-1, accN-1)
#define NUMBER_OF_OPDAT_ARGUMENTS 4
#define BASE_OPDAT_ARG_INDEX 2

// Macros for positions inside the op_decl_set arguments
// we assume the following style:
// op_decl_set ( setCardinality, op_set_ref )
#define OP_DECL_SET_VAR_REF 1

// Macros for positions inside the op_decl_set arguments
// we assume the following style:
// op_decl_map ( fromSet, toSet, cardinality, inputData, op_map_ref )
#define OP_DECL_MAP_VAR_REF 4

// Macros for positions inside the op_decl_set arguments
// we assume the following style:
// op_decl_map ( set, cardinality, inputData, op_dat_ref )
#define OP_DECL_DAT_DIMENSION_VAL 1
#define OP_DECL_DAT_INPUT_REF 2
#define OP_DECL_DAT_VAR_REF 3

#endif
