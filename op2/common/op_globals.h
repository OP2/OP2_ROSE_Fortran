/*
 * Global variable declarations
 */

#ifndef __OP_GLOBALS
#define __OP_GLOBALS

#include "op_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * OP diagnostics level; defined in op_seq.cpp/op_lib.cu and set in op_init
 */

extern int OP_diags;

/*
 * Arrays holding pointers to data structures and their global indices
 */

extern int OP_set_index;
extern int OP_map_index;
extern int OP_dat_index;

extern op_set  **OP_set_list;
extern op_map  **OP_map_list;
extern op_dat  **OP_dat_list;

/*
 * min / max definitions
 */

#ifndef MIN
#define MIN(a,b) ((a<b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a>b) ? (a) : (b))
#endif

/*
 * alignment macro based on example on page 50 of CUDA Programming Guide version 3.0
 * rounds up to nearest multiple of 16 bytes
 */

#define ROUND_UP(bytes) (((bytes) + 15) & ~15)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

