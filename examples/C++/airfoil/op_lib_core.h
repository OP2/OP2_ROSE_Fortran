/*
  Open source copyright declaration based on BSD open source template:
  http://www.opensource.org/licenses/bsd-license.php

* Copyright (c) 2009-2011, Mike Giles
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

//
// header files
//

#include <stdlib.h>                                                         
#include <stdio.h>                                                          
#include <string.h>                                                         
#include <strings.h>                                                         
#include <math.h>                                                           

//
// essential typedefs
//

typedef unsigned int uint;
typedef long long ll;
typedef unsigned long long ull;

//
// OP diagnostics level; defined in op_seq.cpp/op_lib.cu and set in op_init
//

extern int OP_diags;

/*
   0            none
   1 or above   error-checking
   2 or above   info on plan construction
   3 or above   report execution of parallel loops
   4 or above   report use of old plans
   7 or above   report positive checks in op_plan_check
*/

//
// enum list for op_par_loop
//

typedef enum {OP_READ, OP_WRITE, OP_RW, OP_INC, OP_MIN, OP_MAX} op_access;

typedef enum {OP_ARG_GBL, OP_ARG_DAT} op_arg_type;

//
// structures
//

typedef struct {
  int         index;  // index
  int         size;   // number of elements in set
  char const *name;   // name of set
} op_set_core;

typedef op_set_core * op_set;

typedef struct {
  int         index;  // index
  op_set      from,   // set pointed from
              to;     // set pointed to
  int         dim,    // dimension of pointer
             *map;    // array defining pointer
  char const *name;   // name of pointer
} op_map_core;

typedef op_map_core * op_map;

typedef struct {
  int         index;  // index
  op_set      set;    // set on which data is defined
  int         dim,    // dimension of data
              size;   // size of each element in dataset
  char       *data,   // data on host
             *data_d; // data on device (GPU)
  char const *type,   // datatype
             *name;   // name of dataset
} op_dat_core;

typedef op_dat_core * op_dat;

typedef struct {
  int         index;  // index
  op_dat      dat;    // dataset
  op_map      map;    // indirect mapping
  int         dim,    // dimension of data
              idx,    //
              size;   // size (for sequential execution)
  char       *data,   // data on host
             *data_d; // data on device (for CUDA execution)
  char const *type;   // datatype
  op_access   acc;
  op_arg_type argtype;
} op_arg;


typedef struct {
  // input arguments
  char const  *name;
  op_set       set;
  int          nargs, ninds, part_size;
  op_map      *maps;
  op_dat      *dats;
  int         *idxs;
  op_access   *accs;

  // execution plan
  int        *nthrcol;  // number of thread colors for each block
  int        *thrcol;   // thread colors
  int        *offset;   // offset for primary set
  int       **ind_maps; // pointers for indirect datasets
  int        *ind_offs; // block offsets for indirect datasets
  int        *ind_sizes;// block sizes for indirect datasets
  int        *nindirect;// total sizes for indirect datasets
  short     **loc_maps; // maps to local indices, renumbered as needed
  int         nblocks;  // number of blocks
  int        *nelems;   // number of elements in each block
  int         ncolors;  // number of block colors
  int        *ncolblk;  // number of blocks for each color
  int        *blkmap;   // block mapping
  int         nshared;  // bytes of shared memory required
  float       transfer; // bytes of data transfer per kernel call
  float       transfer2;// bytes of cache line per kernel call
  int         count;    // number of times called
} op_plan;

typedef struct {
  char const *name;     // name of kernel function
  int         count;    // number of times called
  float       time;     // total execution time
  float       transfer; // bytes of data transfer (used)
  float       transfer2;// bytes of data transfer (total)
} op_kernel;

//
//  min / max definitions
//

#ifndef MIN
#define MIN(a,b) ((a<b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a>b) ? (a) : (b))
#endif

//
// alignment macro based on example on page 50 of CUDA Programming Guide version 3.0
// rounds up to nearest multiple of 16 bytes
//

#define ROUND_UP(bytes) (((bytes) + 15) & ~15)
