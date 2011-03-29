/*
 * OP runtime library functions
 */

#ifndef __OP_COMMON
#define __OP_COMMON

#include "op_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * OP function prototypes
 */

void op_init(int, char **, int);

void op_exit();

void op_fetch_data(op_dat);

void op_diagnostic_output();

void op_timing_output();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

