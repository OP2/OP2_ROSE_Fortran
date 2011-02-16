/*
 *  OP2CommonDefinitions.h, written by Carlo Bertolli and Adam Betts
 *  
 * This header defines all common configuration parameters of the OP2 API
 *
 */


#ifndef OP2_COMMON_DEFINITIONS_H
#define OP2_COMMON_DEFINITIONS_H


#include <string>

using std::string;


// macros for positions inside the op_par_loop arguments
// we assume the following style:
// op_par_loop_* ( kernel, iteration_set,
//								 arg0, idx0, map0, acc0,
//								 ...
//								 argN-1, idxN-1, mapN-1, accN-1
//							 )
#define ARG_LINE_LENGTH 4
#define OP_SET_INDEX_IN_ARG 1
#define BASE_OPDAT_ARG_INDEX 2


// Number of default configuration parameters used to call CUDA main kernel inside host subroutine
#define CUDA_CONFIG_PARS_NUMBER 6

// macros for positions inside the op_decl_set arguments
// we assume the following style:
// op_decl_set ( setCardinality, op_set_ref )
#define OP_DECL_SET_VAR_REF 1

// macros for positions inside the op_decl_set arguments
// we assume the following style:
// op_decl_map ( fromSet, toSet, cardinality, inputData, op_map_ref )
#define OP_DECL_MAP_VAR_REF 4

// macros for positions inside the op_decl_set arguments
// we assume the following style:
// op_decl_map ( set, cardinality, inputData, op_dat_ref )
#define OP_DECL_DAT_DIMENSION_VAL 1
#define OP_DECL_DAT_INPUT_REF 2
#define OP_DECL_DAT_VAR_REF 3

// declaration of OP2 data type names
const string OP_SET_NAME = "op_set";
const string OP_DAT_NAME = "op_dat";
const string OP_MAP_NAME = "op_map";


#endif