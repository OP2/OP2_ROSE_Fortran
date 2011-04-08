/*
  Open source copyright declaration based on BSD open source template:
  http://www.opensource.org/licenses/bsd-license.php

* Copyright (c) 2009, Mike Giles
* Copyright (c) 2011, Florian Rathgeber
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of Mike Giles may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Mike Giles ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Mike Giles BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * OP datatypes
 */

#ifndef __OP_DATATYPES
#define __OP_DATATYPES

#ifdef __cplusplus
extern "C" {
#else
#include <stdlib.h>
#endif

/*
   0            none
   1 or above   error-checking
   2 or above   info on plan construction
   3 or above   report execution of parallel loops
   4 or above   report use of old plans
   7 or above   report positive checks in op_plan_check
*/

/*
 * enum list for op_par_loop
 */

typedef enum { OP_READ, OP_WRITE, OP_RW, OP_INC, OP_MIN, OP_MAX } op_access;

/*
 * add in user's datatypes
 */

#ifdef OP_USER_DATATYPES
#include <OP_USER_DATATYPES>
#endif

/*
 * zero constants
 */

#define ZERO_double  0.0;
#define ZERO_float   0.0f;
#define ZERO_int     0;
#define ZERO_uint    0;
#define ZERO_ll      0;
#define ZERO_ull     0;
#define ZERO_bool    0;

/*
 * structures
 */

typedef struct {
  int         size,   /* number of elements in set */
              index;  /* index into list of sets  */
  char const *name;   /* name of set */
} op_set;

typedef struct {
  op_set      from,   /* set pointed from */
              to;     /* set pointed to */
  int         dim,    /* dimension of pointer */
              index,  /* index into list of pointers */
             *map;    /* array defining pointer */
  char const *name;   /* name of pointer */
} op_map;

typedef struct {
  op_set      set;    /* set on which data is defined */
  int         dim,    /* dimension of data */
              index,  /* index into list of datasets */
              size;   /* size of each element in dataset */
  void       *dat,    /* data on host */
             *dat_d;  /* data on device (GPU) */
  char const *name;   /* name of dataset */
} op_dat;

typedef struct {
  int         form;
  op_dat     *dat;
  int         idx[2];
  op_map     *map[2];
  op_access   acc;
} op_arg;

/* index for direct mapping */
#define OP_NONE -1
/* all indices selector */
#define OP_ALL -2

/* identity mapping */
#define OP_ID  (op_map) {{0,0,"null"},{0,0,"null"},0,0,NULL,"id"}

/* global identifier */
#define OP_GBL (op_map) {{0,0,"null"},{0,0,"null"},-1,0,NULL,"gbl"}

/* empty set */
#define OP_NULL_SET (op_set) {0, 0, "null_set"}

typedef struct {
  size_t nrows;
  size_t ncols;
  int    *nnz;
  int    *rowptr;
  int    *colidx;
  size_t max_nonzeros;
} op_sparsity;

typedef struct {
  /* input arguments */
  char const  *name;
  int          set_index, nargs;
  int         *arg_idxs, *idxs, *map_idxs, *dims;
  char const **typs;
  op_access   *accs;

  /* execution plan */
  int        *nthrcol;  /* number of thread colors for each block */
  int        *thrcol;   /* thread colors */
  int        *offset;   /* offset for primary set */
  int       **ind_maps; /* pointers for indirect datasets */
  int        *ind_offs; /* offsets for indirect datasets */
  int        *ind_sizes;/* sizes for indirect datasets */
  short     **maps;     /* regular pointers, renumbered as needed */
  int        *nelems;   /* number of elements in each block */
  int         ncolors;  /* number of block colors */
  int        *ncolblk;  /* number of blocks for each color */
  int        *blkmap;   /* block mapping */
  int         nshared;  /* bytes of shared memory required */
  float       transfer; /* bytes of data transfer per kernel call */
  float       transfer2;/* bytes of cache line per kernel call */
} op_plan;

typedef struct {
  char const *name;     /* name of kernel function */
  int         count;    /* number of times called */
  float       time;     /* total execution time */
  float       transfer; /* bytes of data transfer (used) */
  float       transfer2;/* bytes of data transfer (total) */
} op_kernel;


/* 
 * Low-level initialisation functions
 */

void initialise_set ( op_set * set, int size, char const * name );

void initialise_map ( op_map * mapping, op_set * from, op_set * to, int dim, int * map, char const * name );

void initialise_dat ( op_dat * data, op_set * set, int dim, int type_size, void * dat, char const * name );

/*
 * Structure initialisation
 */

void op_decl_set ( op_set * set, int size, char const * name );

void op_decl_map ( op_map * mapping, op_set * from, op_set * to, int dim, int * map, char const * name );

void op_decl_dat ( op_dat * data, op_set * set, int dim, int type_size, void * dat, char const * name );

void op_create_vec ( op_dat * vec );

void op_decl_id_map ( op_map * map );

void op_decl_gbl_map ( op_map * map );

void op_decl_const ( void * dat, int dim, int type_size );

op_arg op_construct_gbl_arg(op_dat * data, op_access acc);

op_arg op_construct_vec_arg(op_dat * data, int idx, op_map * mapping, op_access acc);

op_arg op_construct_mat_arg(op_dat * data, int idx0, op_map * map0, int idx1, op_map * map1, op_access acc);

void op_decl_sparsity ( op_sparsity * sparsity, op_map * rowmap, op_map * colmap );

void op_decl_mat( op_dat * mat, op_sparsity * sparsity, char const * name );

void op_mat_addto( op_dat * mat, const void* values, int nrows, const int *irows, int ncols, const int *icols );

void op_mat_assemble( op_dat * mat );

void op_mat_mult ( const op_dat * mat, const op_dat * v_in, op_dat * v_out );

void dump_set ( op_set const * set, char const * filename );

void dump_map ( op_map const * map, char const * filename );

void dump_dat ( op_dat const * dat, char const * filename );

void dump_sparsity ( op_sparsity const * sparsity, char const * filename );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

