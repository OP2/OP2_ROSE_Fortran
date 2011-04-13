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
 * header files
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <op2/common/op_par_loop.h>

static inline void arg_check(op_set * set, int argnum, op_arg * argument, const char * kernel) {

  for (int i = 0; i < argument->form; ++i) {
    if (argument->map[i] == NULL) {
      if (argument->idx[i] != OP_NONE) {
        printf("error: arg %d in kernel %s\n", argnum, kernel);
        printf("invalid index, should be OP_NONE for identity mapping %d\n", i);
        exit(1);
      }
    }
    else {
      if (set->index != argument->map[i]->from->index
          || (argument->dat->set && argument->dat->set->index != argument->map[i]->to->index)) {
        printf("error: arg %d in kernel %s\n", argnum, kernel);
        printf("invalid mapping %d\n", i);
        exit(1);
      }
      if (argument->idx[i] != OP_ALL && (argument->idx[i] < 0 || argument->idx[i] >= argument->map[i]->dim)) {
        printf("error: arg %d in kernel %s\n", argnum, kernel);
        printf("invalid index into mapping %d\n", i);
        exit(1);
      }
    }
  }
}

static inline void arg_set (int displacement, op_arg * argument, char ** p_arg)
{
  int n2;

  if ( argument->map[0] == 0 )			// identity mapping
		n2 = displacement;
  else if ( argument->map[0]->dim > 0 ) // standard pointers
    n2 = argument->map[0]->map[argument->idx[0] + displacement * argument->map[0]->dim];
  else
    printf("error: arg_set failed for set element %d (don't know what to do)\n", displacement);

  *p_arg = (char*) argument->dat->dat + n2 * argument->dat->size;
}

static inline void copy_in(int n, op_dat * arg, op_map * map, char *p_arg){
  // For each index in the target dimension of map, copy the chunk of data for
  // the current element
  for (int i = 0; i < map->dim; ++i) {
      memcpy(p_arg+i*arg->size, (char*) arg->dat + map->map[i+n*map->dim]*arg->size, arg->size);
  }
}

static inline void copy_out(int n, op_dat * arg, op_map * map, char *p_arg){
  // For each index in the target dimension of map, copy the chunk of data for
  // the current element
  for (int i = 0; i < map->dim; ++i) {
      memcpy((char*) arg->dat + map->map[i+n*map->dim]*arg->size, p_arg+i*arg->size, arg->size);
  }
}

void op_par_loop_1 ( void (*kernel)(void *), char const * name, op_set * set,
                     op_arg arg0
                   )

{
  arg_check ( set, 0, &arg0 , name );

  char * ptr0 = 0;

  switch( arg0.form ) {
  case 0:
    ptr0 = (char*) arg0.dat->dat;
    break;
  case 1:
    if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
    break;
  case 2:
    ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.map[1]->dim * arg0.dat->size);
    break;
  }


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.form == 1) {
      if (arg0.idx[0]  == OP_ALL) {
        if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
          copy_in(i, arg0.dat, arg0.map[0], ptr0 );
      } else {
        arg_set ( i, &arg0, &ptr0 );
      }
    }

    kernel(ptr0);

    if (arg0.form == 1 && arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    } else if (arg0.form == 2) {
      const int rows = arg0.map[0]->dim;
      const int cols = arg0.map[1]->dim;
      op_mat_addto( arg0.dat, ptr0, rows, arg0.map[0]->map + i*rows, cols, arg0.map[1]->map + i*cols);
    }

  }

  if ((arg0.form == 1 && arg0.idx[0]  == OP_ALL) || arg0.form == 2) free(ptr0);
  if (arg0.form == 2) op_mat_assemble(arg0.dat);

}

void op_par_loop_2 ( void (*kernel)(void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1
                   )

{
  arg_check ( set, 0, &arg0 , name );
  arg_check ( set, 1, &arg1 , name );

  char * ptr0 = 0, * ptr1 = 0;

  switch( arg0.form ) {
  case 0:
    ptr0 = (char*) arg0.dat->dat;
    break;
  case 1:
    if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
    break;
  case 2:
    ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.map[1]->dim * arg0.dat->size);
    break;
  }

  switch( arg1.form ) {
  case 0:
    ptr1 = (char*) arg1.dat->dat;
    break;
  case 1:
    if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
    break;
  case 2:
    ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.map[1]->dim * arg1.dat->size);
    break;
  }


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.form == 1) {
      if (arg0.idx[0]  == OP_ALL) {
        if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
          copy_in(i, arg0.dat, arg0.map[0], ptr0 );
      } else {
        arg_set ( i, &arg0, &ptr0 );
      }
    }

    if (arg1.form == 1) {
      if (arg1.idx[0]  == OP_ALL) {
        if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
          copy_in(i, arg1.dat, arg1.map[0], ptr1 );
      } else {
        arg_set ( i, &arg1, &ptr1 );
      }
    }

    kernel(ptr0, ptr1);

    if (arg0.form == 1 && arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    } else if (arg0.form == 2) {
      const int rows = arg0.map[0]->dim;
      const int cols = arg0.map[1]->dim;
      op_mat_addto( arg0.dat, ptr0, rows, arg0.map[0]->map + i*rows, cols, arg0.map[1]->map + i*cols);
    }

    if (arg1.form == 1 && arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    } else if (arg1.form == 2) {
      const int rows = arg1.map[0]->dim;
      const int cols = arg1.map[1]->dim;
      op_mat_addto( arg1.dat, ptr1, rows, arg1.map[0]->map + i*rows, cols, arg1.map[1]->map + i*cols);
    }

  }

  if ((arg0.form == 1 && arg0.idx[0]  == OP_ALL) || arg0.form == 2) free(ptr0);
  if (arg0.form == 2) op_mat_assemble(arg0.dat);
  if ((arg1.form == 1 && arg1.idx[0]  == OP_ALL) || arg1.form == 2) free(ptr1);
  if (arg1.form == 2) op_mat_assemble(arg1.dat);

}

void op_par_loop_3 ( void (*kernel)(void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2
                   )

{
  arg_check ( set, 0, &arg0 , name );
  arg_check ( set, 1, &arg1 , name );
  arg_check ( set, 2, &arg2 , name );

  char * ptr0 = 0, * ptr1 = 0, * ptr2 = 0;

  switch( arg0.form ) {
  case 0:
    ptr0 = (char*) arg0.dat->dat;
    break;
  case 1:
    if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
    break;
  case 2:
    ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.map[1]->dim * arg0.dat->size);
    break;
  }

  switch( arg1.form ) {
  case 0:
    ptr1 = (char*) arg1.dat->dat;
    break;
  case 1:
    if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
    break;
  case 2:
    ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.map[1]->dim * arg1.dat->size);
    break;
  }

  switch( arg2.form ) {
  case 0:
    ptr2 = (char*) arg2.dat->dat;
    break;
  case 1:
    if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);
    break;
  case 2:
    ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.map[1]->dim * arg2.dat->size);
    break;
  }


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.form == 1) {
      if (arg0.idx[0]  == OP_ALL) {
        if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
          copy_in(i, arg0.dat, arg0.map[0], ptr0 );
      } else {
        arg_set ( i, &arg0, &ptr0 );
      }
    }

    if (arg1.form == 1) {
      if (arg1.idx[0]  == OP_ALL) {
        if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
          copy_in(i, arg1.dat, arg1.map[0], ptr1 );
      } else {
        arg_set ( i, &arg1, &ptr1 );
      }
    }

    if (arg2.form == 1) {
      if (arg2.idx[0]  == OP_ALL) {
        if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
          copy_in(i, arg2.dat, arg2.map[0], ptr2 );
      } else {
        arg_set ( i, &arg2, &ptr2 );
      }
    }

    kernel(ptr0, ptr1, ptr2);

    if (arg0.form == 1 && arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    } else if (arg0.form == 2) {
      const int rows = arg0.map[0]->dim;
      const int cols = arg0.map[1]->dim;
      op_mat_addto( arg0.dat, ptr0, rows, arg0.map[0]->map + i*rows, cols, arg0.map[1]->map + i*cols);
    }

    if (arg1.form == 1 && arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    } else if (arg1.form == 2) {
      const int rows = arg1.map[0]->dim;
      const int cols = arg1.map[1]->dim;
      op_mat_addto( arg1.dat, ptr1, rows, arg1.map[0]->map + i*rows, cols, arg1.map[1]->map + i*cols);
    }

    if (arg2.form == 1 && arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
    } else if (arg2.form == 2) {
      const int rows = arg2.map[0]->dim;
      const int cols = arg2.map[1]->dim;
      op_mat_addto( arg2.dat, ptr2, rows, arg2.map[0]->map + i*rows, cols, arg2.map[1]->map + i*cols);
    }

  }

  if ((arg0.form == 1 && arg0.idx[0]  == OP_ALL) || arg0.form == 2) free(ptr0);
  if (arg0.form == 2) op_mat_assemble(arg0.dat);
  if ((arg1.form == 1 && arg1.idx[0]  == OP_ALL) || arg1.form == 2) free(ptr1);
  if (arg1.form == 2) op_mat_assemble(arg1.dat);
  if ((arg2.form == 1 && arg2.idx[0]  == OP_ALL) || arg2.form == 2) free(ptr2);
  if (arg2.form == 2) op_mat_assemble(arg2.dat);

}

void op_par_loop_4 ( void (*kernel)(void *, void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2,
                     op_arg arg3
                   )

{
  arg_check ( set, 0, &arg0 , name );
  arg_check ( set, 1, &arg1 , name );
  arg_check ( set, 2, &arg2 , name );
  arg_check ( set, 3, &arg3 , name );

  char * ptr0 = 0, * ptr1 = 0, * ptr2 = 0, * ptr3 = 0;

  switch( arg0.form ) {
  case 0:
    ptr0 = (char*) arg0.dat->dat;
    break;
  case 1:
    if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
    break;
  case 2:
    ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.map[1]->dim * arg0.dat->size);
    break;
  }

  switch( arg1.form ) {
  case 0:
    ptr1 = (char*) arg1.dat->dat;
    break;
  case 1:
    if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
    break;
  case 2:
    ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.map[1]->dim * arg1.dat->size);
    break;
  }

  switch( arg2.form ) {
  case 0:
    ptr2 = (char*) arg2.dat->dat;
    break;
  case 1:
    if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);
    break;
  case 2:
    ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.map[1]->dim * arg2.dat->size);
    break;
  }

  switch( arg3.form ) {
  case 0:
    ptr3 = (char*) arg3.dat->dat;
    break;
  case 1:
    if (arg3.idx[0]  == OP_ALL) ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.dat->size);
    break;
  case 2:
    ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.map[1]->dim * arg3.dat->size);
    break;
  }


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.form == 1) {
      if (arg0.idx[0]  == OP_ALL) {
        if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
          copy_in(i, arg0.dat, arg0.map[0], ptr0 );
      } else {
        arg_set ( i, &arg0, &ptr0 );
      }
    }

    if (arg1.form == 1) {
      if (arg1.idx[0]  == OP_ALL) {
        if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
          copy_in(i, arg1.dat, arg1.map[0], ptr1 );
      } else {
        arg_set ( i, &arg1, &ptr1 );
      }
    }

    if (arg2.form == 1) {
      if (arg2.idx[0]  == OP_ALL) {
        if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
          copy_in(i, arg2.dat, arg2.map[0], ptr2 );
      } else {
        arg_set ( i, &arg2, &ptr2 );
      }
    }

    if (arg3.form == 1) {
      if (arg3.idx[0]  == OP_ALL) {
        if (arg3.acc  == OP_READ || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
          copy_in(i, arg3.dat, arg3.map[0], ptr3 );
      } else {
        arg_set ( i, &arg3, &ptr3 );
      }
    }

    kernel(ptr0, ptr1, ptr2, ptr3);

    if (arg0.form == 1 && arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    } else if (arg0.form == 2) {
      const int rows = arg0.map[0]->dim;
      const int cols = arg0.map[1]->dim;
      op_mat_addto( arg0.dat, ptr0, rows, arg0.map[0]->map + i*rows, cols, arg0.map[1]->map + i*cols);
    }

    if (arg1.form == 1 && arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    } else if (arg1.form == 2) {
      const int rows = arg1.map[0]->dim;
      const int cols = arg1.map[1]->dim;
      op_mat_addto( arg1.dat, ptr1, rows, arg1.map[0]->map + i*rows, cols, arg1.map[1]->map + i*cols);
    }

    if (arg2.form == 1 && arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
    } else if (arg2.form == 2) {
      const int rows = arg2.map[0]->dim;
      const int cols = arg2.map[1]->dim;
      op_mat_addto( arg2.dat, ptr2, rows, arg2.map[0]->map + i*rows, cols, arg2.map[1]->map + i*cols);
    }

    if (arg3.form == 1 && arg3.idx[0]  == OP_ALL) {
      if (arg3.acc  == OP_WRITE || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_out(i, arg3.dat, arg3.map[0], ptr3 );
    } else if (arg3.form == 2) {
      const int rows = arg3.map[0]->dim;
      const int cols = arg3.map[1]->dim;
      op_mat_addto( arg3.dat, ptr3, rows, arg3.map[0]->map + i*rows, cols, arg3.map[1]->map + i*cols);
    }

  }

  if ((arg0.form == 1 && arg0.idx[0]  == OP_ALL) || arg0.form == 2) free(ptr0);
  if (arg0.form == 2) op_mat_assemble(arg0.dat);
  if ((arg1.form == 1 && arg1.idx[0]  == OP_ALL) || arg1.form == 2) free(ptr1);
  if (arg1.form == 2) op_mat_assemble(arg1.dat);
  if ((arg2.form == 1 && arg2.idx[0]  == OP_ALL) || arg2.form == 2) free(ptr2);
  if (arg2.form == 2) op_mat_assemble(arg2.dat);
  if ((arg3.form == 1 && arg3.idx[0]  == OP_ALL) || arg3.form == 2) free(ptr3);
  if (arg3.form == 2) op_mat_assemble(arg3.dat);

}

void op_par_loop_5 ( void (*kernel)(void *, void *, void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2,
                     op_arg arg3,
                     op_arg arg4
                   )

{
  arg_check ( set, 0, &arg0 , name );
  arg_check ( set, 1, &arg1 , name );
  arg_check ( set, 2, &arg2 , name );
  arg_check ( set, 3, &arg3 , name );
  arg_check ( set, 4, &arg4 , name );

  char * ptr0 = 0, * ptr1 = 0, * ptr2 = 0, * ptr3 = 0, * ptr4 = 0;

  switch( arg0.form ) {
  case 0:
    ptr0 = (char*) arg0.dat->dat;
    break;
  case 1:
    if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
    break;
  case 2:
    ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.map[1]->dim * arg0.dat->size);
    break;
  }

  switch( arg1.form ) {
  case 0:
    ptr1 = (char*) arg1.dat->dat;
    break;
  case 1:
    if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
    break;
  case 2:
    ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.map[1]->dim * arg1.dat->size);
    break;
  }

  switch( arg2.form ) {
  case 0:
    ptr2 = (char*) arg2.dat->dat;
    break;
  case 1:
    if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);
    break;
  case 2:
    ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.map[1]->dim * arg2.dat->size);
    break;
  }

  switch( arg3.form ) {
  case 0:
    ptr3 = (char*) arg3.dat->dat;
    break;
  case 1:
    if (arg3.idx[0]  == OP_ALL) ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.dat->size);
    break;
  case 2:
    ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.map[1]->dim * arg3.dat->size);
    break;
  }

  switch( arg4.form ) {
  case 0:
    ptr4 = (char*) arg4.dat->dat;
    break;
  case 1:
    if (arg4.idx[0]  == OP_ALL) ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.dat->size);
    break;
  case 2:
    ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.map[1]->dim * arg4.dat->size);
    break;
  }


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.form == 1) {
      if (arg0.idx[0]  == OP_ALL) {
        if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
          copy_in(i, arg0.dat, arg0.map[0], ptr0 );
      } else {
        arg_set ( i, &arg0, &ptr0 );
      }
    }

    if (arg1.form == 1) {
      if (arg1.idx[0]  == OP_ALL) {
        if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
          copy_in(i, arg1.dat, arg1.map[0], ptr1 );
      } else {
        arg_set ( i, &arg1, &ptr1 );
      }
    }

    if (arg2.form == 1) {
      if (arg2.idx[0]  == OP_ALL) {
        if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
          copy_in(i, arg2.dat, arg2.map[0], ptr2 );
      } else {
        arg_set ( i, &arg2, &ptr2 );
      }
    }

    if (arg3.form == 1) {
      if (arg3.idx[0]  == OP_ALL) {
        if (arg3.acc  == OP_READ || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
          copy_in(i, arg3.dat, arg3.map[0], ptr3 );
      } else {
        arg_set ( i, &arg3, &ptr3 );
      }
    }

    if (arg4.form == 1) {
      if (arg4.idx[0]  == OP_ALL) {
        if (arg4.acc  == OP_READ || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
          copy_in(i, arg4.dat, arg4.map[0], ptr4 );
      } else {
        arg_set ( i, &arg4, &ptr4 );
      }
    }

    kernel(ptr0, ptr1, ptr2, ptr3, ptr4);

    if (arg0.form == 1 && arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    } else if (arg0.form == 2) {
      const int rows = arg0.map[0]->dim;
      const int cols = arg0.map[1]->dim;
      op_mat_addto( arg0.dat, ptr0, rows, arg0.map[0]->map + i*rows, cols, arg0.map[1]->map + i*cols);
    }

    if (arg1.form == 1 && arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    } else if (arg1.form == 2) {
      const int rows = arg1.map[0]->dim;
      const int cols = arg1.map[1]->dim;
      op_mat_addto( arg1.dat, ptr1, rows, arg1.map[0]->map + i*rows, cols, arg1.map[1]->map + i*cols);
    }

    if (arg2.form == 1 && arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
    } else if (arg2.form == 2) {
      const int rows = arg2.map[0]->dim;
      const int cols = arg2.map[1]->dim;
      op_mat_addto( arg2.dat, ptr2, rows, arg2.map[0]->map + i*rows, cols, arg2.map[1]->map + i*cols);
    }

    if (arg3.form == 1 && arg3.idx[0]  == OP_ALL) {
      if (arg3.acc  == OP_WRITE || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_out(i, arg3.dat, arg3.map[0], ptr3 );
    } else if (arg3.form == 2) {
      const int rows = arg3.map[0]->dim;
      const int cols = arg3.map[1]->dim;
      op_mat_addto( arg3.dat, ptr3, rows, arg3.map[0]->map + i*rows, cols, arg3.map[1]->map + i*cols);
    }

    if (arg4.form == 1 && arg4.idx[0]  == OP_ALL) {
      if (arg4.acc  == OP_WRITE || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_out(i, arg4.dat, arg4.map[0], ptr4 );
    } else if (arg4.form == 2) {
      const int rows = arg4.map[0]->dim;
      const int cols = arg4.map[1]->dim;
      op_mat_addto( arg4.dat, ptr4, rows, arg4.map[0]->map + i*rows, cols, arg4.map[1]->map + i*cols);
    }

  }

  if ((arg0.form == 1 && arg0.idx[0]  == OP_ALL) || arg0.form == 2) free(ptr0);
  if (arg0.form == 2) op_mat_assemble(arg0.dat);
  if ((arg1.form == 1 && arg1.idx[0]  == OP_ALL) || arg1.form == 2) free(ptr1);
  if (arg1.form == 2) op_mat_assemble(arg1.dat);
  if ((arg2.form == 1 && arg2.idx[0]  == OP_ALL) || arg2.form == 2) free(ptr2);
  if (arg2.form == 2) op_mat_assemble(arg2.dat);
  if ((arg3.form == 1 && arg3.idx[0]  == OP_ALL) || arg3.form == 2) free(ptr3);
  if (arg3.form == 2) op_mat_assemble(arg3.dat);
  if ((arg4.form == 1 && arg4.idx[0]  == OP_ALL) || arg4.form == 2) free(ptr4);
  if (arg4.form == 2) op_mat_assemble(arg4.dat);

}

void op_par_loop_6 ( void (*kernel)(void *, void *, void *, void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2,
                     op_arg arg3,
                     op_arg arg4,
                     op_arg arg5
                   )

{
  arg_check ( set, 0, &arg0 , name );
  arg_check ( set, 1, &arg1 , name );
  arg_check ( set, 2, &arg2 , name );
  arg_check ( set, 3, &arg3 , name );
  arg_check ( set, 4, &arg4 , name );
  arg_check ( set, 5, &arg5 , name );

  char * ptr0 = 0, * ptr1 = 0, * ptr2 = 0, * ptr3 = 0, * ptr4 = 0, * ptr5 = 0;

  switch( arg0.form ) {
  case 0:
    ptr0 = (char*) arg0.dat->dat;
    break;
  case 1:
    if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
    break;
  case 2:
    ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.map[1]->dim * arg0.dat->size);
    break;
  }

  switch( arg1.form ) {
  case 0:
    ptr1 = (char*) arg1.dat->dat;
    break;
  case 1:
    if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
    break;
  case 2:
    ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.map[1]->dim * arg1.dat->size);
    break;
  }

  switch( arg2.form ) {
  case 0:
    ptr2 = (char*) arg2.dat->dat;
    break;
  case 1:
    if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);
    break;
  case 2:
    ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.map[1]->dim * arg2.dat->size);
    break;
  }

  switch( arg3.form ) {
  case 0:
    ptr3 = (char*) arg3.dat->dat;
    break;
  case 1:
    if (arg3.idx[0]  == OP_ALL) ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.dat->size);
    break;
  case 2:
    ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.map[1]->dim * arg3.dat->size);
    break;
  }

  switch( arg4.form ) {
  case 0:
    ptr4 = (char*) arg4.dat->dat;
    break;
  case 1:
    if (arg4.idx[0]  == OP_ALL) ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.dat->size);
    break;
  case 2:
    ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.map[1]->dim * arg4.dat->size);
    break;
  }

  switch( arg5.form ) {
  case 0:
    ptr5 = (char*) arg5.dat->dat;
    break;
  case 1:
    if (arg5.idx[0]  == OP_ALL) ptr5 = (char*) malloc(arg5.map[0]->dim * arg5.dat->size);
    break;
  case 2:
    ptr5 = (char*) malloc(arg5.map[0]->dim * arg5.map[1]->dim * arg5.dat->size);
    break;
  }


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.form == 1) {
      if (arg0.idx[0]  == OP_ALL) {
        if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
          copy_in(i, arg0.dat, arg0.map[0], ptr0 );
      } else {
        arg_set ( i, &arg0, &ptr0 );
      }
    }

    if (arg1.form == 1) {
      if (arg1.idx[0]  == OP_ALL) {
        if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
          copy_in(i, arg1.dat, arg1.map[0], ptr1 );
      } else {
        arg_set ( i, &arg1, &ptr1 );
      }
    }

    if (arg2.form == 1) {
      if (arg2.idx[0]  == OP_ALL) {
        if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
          copy_in(i, arg2.dat, arg2.map[0], ptr2 );
      } else {
        arg_set ( i, &arg2, &ptr2 );
      }
    }

    if (arg3.form == 1) {
      if (arg3.idx[0]  == OP_ALL) {
        if (arg3.acc  == OP_READ || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
          copy_in(i, arg3.dat, arg3.map[0], ptr3 );
      } else {
        arg_set ( i, &arg3, &ptr3 );
      }
    }

    if (arg4.form == 1) {
      if (arg4.idx[0]  == OP_ALL) {
        if (arg4.acc  == OP_READ || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
          copy_in(i, arg4.dat, arg4.map[0], ptr4 );
      } else {
        arg_set ( i, &arg4, &ptr4 );
      }
    }

    if (arg5.form == 1) {
      if (arg5.idx[0]  == OP_ALL) {
        if (arg5.acc  == OP_READ || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
          copy_in(i, arg5.dat, arg5.map[0], ptr5 );
      } else {
        arg_set ( i, &arg5, &ptr5 );
      }
    }

    kernel(ptr0, ptr1, ptr2, ptr3, ptr4, ptr5);

    if (arg0.form == 1 && arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    } else if (arg0.form == 2) {
      const int rows = arg0.map[0]->dim;
      const int cols = arg0.map[1]->dim;
      op_mat_addto( arg0.dat, ptr0, rows, arg0.map[0]->map + i*rows, cols, arg0.map[1]->map + i*cols);
    }

    if (arg1.form == 1 && arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    } else if (arg1.form == 2) {
      const int rows = arg1.map[0]->dim;
      const int cols = arg1.map[1]->dim;
      op_mat_addto( arg1.dat, ptr1, rows, arg1.map[0]->map + i*rows, cols, arg1.map[1]->map + i*cols);
    }

    if (arg2.form == 1 && arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
    } else if (arg2.form == 2) {
      const int rows = arg2.map[0]->dim;
      const int cols = arg2.map[1]->dim;
      op_mat_addto( arg2.dat, ptr2, rows, arg2.map[0]->map + i*rows, cols, arg2.map[1]->map + i*cols);
    }

    if (arg3.form == 1 && arg3.idx[0]  == OP_ALL) {
      if (arg3.acc  == OP_WRITE || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_out(i, arg3.dat, arg3.map[0], ptr3 );
    } else if (arg3.form == 2) {
      const int rows = arg3.map[0]->dim;
      const int cols = arg3.map[1]->dim;
      op_mat_addto( arg3.dat, ptr3, rows, arg3.map[0]->map + i*rows, cols, arg3.map[1]->map + i*cols);
    }

    if (arg4.form == 1 && arg4.idx[0]  == OP_ALL) {
      if (arg4.acc  == OP_WRITE || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_out(i, arg4.dat, arg4.map[0], ptr4 );
    } else if (arg4.form == 2) {
      const int rows = arg4.map[0]->dim;
      const int cols = arg4.map[1]->dim;
      op_mat_addto( arg4.dat, ptr4, rows, arg4.map[0]->map + i*rows, cols, arg4.map[1]->map + i*cols);
    }

    if (arg5.form == 1 && arg5.idx[0]  == OP_ALL) {
      if (arg5.acc  == OP_WRITE || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
        copy_out(i, arg5.dat, arg5.map[0], ptr5 );
    } else if (arg5.form == 2) {
      const int rows = arg5.map[0]->dim;
      const int cols = arg5.map[1]->dim;
      op_mat_addto( arg5.dat, ptr5, rows, arg5.map[0]->map + i*rows, cols, arg5.map[1]->map + i*cols);
    }

  }

  if ((arg0.form == 1 && arg0.idx[0]  == OP_ALL) || arg0.form == 2) free(ptr0);
  if (arg0.form == 2) op_mat_assemble(arg0.dat);
  if ((arg1.form == 1 && arg1.idx[0]  == OP_ALL) || arg1.form == 2) free(ptr1);
  if (arg1.form == 2) op_mat_assemble(arg1.dat);
  if ((arg2.form == 1 && arg2.idx[0]  == OP_ALL) || arg2.form == 2) free(ptr2);
  if (arg2.form == 2) op_mat_assemble(arg2.dat);
  if ((arg3.form == 1 && arg3.idx[0]  == OP_ALL) || arg3.form == 2) free(ptr3);
  if (arg3.form == 2) op_mat_assemble(arg3.dat);
  if ((arg4.form == 1 && arg4.idx[0]  == OP_ALL) || arg4.form == 2) free(ptr4);
  if (arg4.form == 2) op_mat_assemble(arg4.dat);
  if ((arg5.form == 1 && arg5.idx[0]  == OP_ALL) || arg5.form == 2) free(ptr5);
  if (arg5.form == 2) op_mat_assemble(arg5.dat);

}

void op_par_loop_7 ( void (*kernel)(void *, void *, void *, void *, void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2,
                     op_arg arg3,
                     op_arg arg4,
                     op_arg arg5,
                     op_arg arg6
                   )

{
  arg_check ( set, 0, &arg0 , name );
  arg_check ( set, 1, &arg1 , name );
  arg_check ( set, 2, &arg2 , name );
  arg_check ( set, 3, &arg3 , name );
  arg_check ( set, 4, &arg4 , name );
  arg_check ( set, 5, &arg5 , name );
  arg_check ( set, 6, &arg6 , name );

  char * ptr0 = 0, * ptr1 = 0, * ptr2 = 0, * ptr3 = 0, * ptr4 = 0, * ptr5 = 0, * ptr6 = 0;

  switch( arg0.form ) {
  case 0:
    ptr0 = (char*) arg0.dat->dat;
    break;
  case 1:
    if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
    break;
  case 2:
    ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.map[1]->dim * arg0.dat->size);
    break;
  }

  switch( arg1.form ) {
  case 0:
    ptr1 = (char*) arg1.dat->dat;
    break;
  case 1:
    if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
    break;
  case 2:
    ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.map[1]->dim * arg1.dat->size);
    break;
  }

  switch( arg2.form ) {
  case 0:
    ptr2 = (char*) arg2.dat->dat;
    break;
  case 1:
    if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);
    break;
  case 2:
    ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.map[1]->dim * arg2.dat->size);
    break;
  }

  switch( arg3.form ) {
  case 0:
    ptr3 = (char*) arg3.dat->dat;
    break;
  case 1:
    if (arg3.idx[0]  == OP_ALL) ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.dat->size);
    break;
  case 2:
    ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.map[1]->dim * arg3.dat->size);
    break;
  }

  switch( arg4.form ) {
  case 0:
    ptr4 = (char*) arg4.dat->dat;
    break;
  case 1:
    if (arg4.idx[0]  == OP_ALL) ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.dat->size);
    break;
  case 2:
    ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.map[1]->dim * arg4.dat->size);
    break;
  }

  switch( arg5.form ) {
  case 0:
    ptr5 = (char*) arg5.dat->dat;
    break;
  case 1:
    if (arg5.idx[0]  == OP_ALL) ptr5 = (char*) malloc(arg5.map[0]->dim * arg5.dat->size);
    break;
  case 2:
    ptr5 = (char*) malloc(arg5.map[0]->dim * arg5.map[1]->dim * arg5.dat->size);
    break;
  }

  switch( arg6.form ) {
  case 0:
    ptr6 = (char*) arg6.dat->dat;
    break;
  case 1:
    if (arg6.idx[0]  == OP_ALL) ptr6 = (char*) malloc(arg6.map[0]->dim * arg6.dat->size);
    break;
  case 2:
    ptr6 = (char*) malloc(arg6.map[0]->dim * arg6.map[1]->dim * arg6.dat->size);
    break;
  }


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.form == 1) {
      if (arg0.idx[0]  == OP_ALL) {
        if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
          copy_in(i, arg0.dat, arg0.map[0], ptr0 );
      } else {
        arg_set ( i, &arg0, &ptr0 );
      }
    }

    if (arg1.form == 1) {
      if (arg1.idx[0]  == OP_ALL) {
        if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
          copy_in(i, arg1.dat, arg1.map[0], ptr1 );
      } else {
        arg_set ( i, &arg1, &ptr1 );
      }
    }

    if (arg2.form == 1) {
      if (arg2.idx[0]  == OP_ALL) {
        if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
          copy_in(i, arg2.dat, arg2.map[0], ptr2 );
      } else {
        arg_set ( i, &arg2, &ptr2 );
      }
    }

    if (arg3.form == 1) {
      if (arg3.idx[0]  == OP_ALL) {
        if (arg3.acc  == OP_READ || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
          copy_in(i, arg3.dat, arg3.map[0], ptr3 );
      } else {
        arg_set ( i, &arg3, &ptr3 );
      }
    }

    if (arg4.form == 1) {
      if (arg4.idx[0]  == OP_ALL) {
        if (arg4.acc  == OP_READ || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
          copy_in(i, arg4.dat, arg4.map[0], ptr4 );
      } else {
        arg_set ( i, &arg4, &ptr4 );
      }
    }

    if (arg5.form == 1) {
      if (arg5.idx[0]  == OP_ALL) {
        if (arg5.acc  == OP_READ || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
          copy_in(i, arg5.dat, arg5.map[0], ptr5 );
      } else {
        arg_set ( i, &arg5, &ptr5 );
      }
    }

    if (arg6.form == 1) {
      if (arg6.idx[0]  == OP_ALL) {
        if (arg6.acc  == OP_READ || arg6.acc  == OP_RW || arg6.acc  == OP_INC)
          copy_in(i, arg6.dat, arg6.map[0], ptr6 );
      } else {
        arg_set ( i, &arg6, &ptr6 );
      }
    }

    kernel(ptr0, ptr1, ptr2, ptr3, ptr4, ptr5, ptr6);

    if (arg0.form == 1 && arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    } else if (arg0.form == 2) {
      const int rows = arg0.map[0]->dim;
      const int cols = arg0.map[1]->dim;
      op_mat_addto( arg0.dat, ptr0, rows, arg0.map[0]->map + i*rows, cols, arg0.map[1]->map + i*cols);
    }

    if (arg1.form == 1 && arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    } else if (arg1.form == 2) {
      const int rows = arg1.map[0]->dim;
      const int cols = arg1.map[1]->dim;
      op_mat_addto( arg1.dat, ptr1, rows, arg1.map[0]->map + i*rows, cols, arg1.map[1]->map + i*cols);
    }

    if (arg2.form == 1 && arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
    } else if (arg2.form == 2) {
      const int rows = arg2.map[0]->dim;
      const int cols = arg2.map[1]->dim;
      op_mat_addto( arg2.dat, ptr2, rows, arg2.map[0]->map + i*rows, cols, arg2.map[1]->map + i*cols);
    }

    if (arg3.form == 1 && arg3.idx[0]  == OP_ALL) {
      if (arg3.acc  == OP_WRITE || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_out(i, arg3.dat, arg3.map[0], ptr3 );
    } else if (arg3.form == 2) {
      const int rows = arg3.map[0]->dim;
      const int cols = arg3.map[1]->dim;
      op_mat_addto( arg3.dat, ptr3, rows, arg3.map[0]->map + i*rows, cols, arg3.map[1]->map + i*cols);
    }

    if (arg4.form == 1 && arg4.idx[0]  == OP_ALL) {
      if (arg4.acc  == OP_WRITE || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_out(i, arg4.dat, arg4.map[0], ptr4 );
    } else if (arg4.form == 2) {
      const int rows = arg4.map[0]->dim;
      const int cols = arg4.map[1]->dim;
      op_mat_addto( arg4.dat, ptr4, rows, arg4.map[0]->map + i*rows, cols, arg4.map[1]->map + i*cols);
    }

    if (arg5.form == 1 && arg5.idx[0]  == OP_ALL) {
      if (arg5.acc  == OP_WRITE || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
        copy_out(i, arg5.dat, arg5.map[0], ptr5 );
    } else if (arg5.form == 2) {
      const int rows = arg5.map[0]->dim;
      const int cols = arg5.map[1]->dim;
      op_mat_addto( arg5.dat, ptr5, rows, arg5.map[0]->map + i*rows, cols, arg5.map[1]->map + i*cols);
    }

    if (arg6.form == 1 && arg6.idx[0]  == OP_ALL) {
      if (arg6.acc  == OP_WRITE || arg6.acc  == OP_RW || arg6.acc  == OP_INC)
        copy_out(i, arg6.dat, arg6.map[0], ptr6 );
    } else if (arg6.form == 2) {
      const int rows = arg6.map[0]->dim;
      const int cols = arg6.map[1]->dim;
      op_mat_addto( arg6.dat, ptr6, rows, arg6.map[0]->map + i*rows, cols, arg6.map[1]->map + i*cols);
    }

  }

  if ((arg0.form == 1 && arg0.idx[0]  == OP_ALL) || arg0.form == 2) free(ptr0);
  if (arg0.form == 2) op_mat_assemble(arg0.dat);
  if ((arg1.form == 1 && arg1.idx[0]  == OP_ALL) || arg1.form == 2) free(ptr1);
  if (arg1.form == 2) op_mat_assemble(arg1.dat);
  if ((arg2.form == 1 && arg2.idx[0]  == OP_ALL) || arg2.form == 2) free(ptr2);
  if (arg2.form == 2) op_mat_assemble(arg2.dat);
  if ((arg3.form == 1 && arg3.idx[0]  == OP_ALL) || arg3.form == 2) free(ptr3);
  if (arg3.form == 2) op_mat_assemble(arg3.dat);
  if ((arg4.form == 1 && arg4.idx[0]  == OP_ALL) || arg4.form == 2) free(ptr4);
  if (arg4.form == 2) op_mat_assemble(arg4.dat);
  if ((arg5.form == 1 && arg5.idx[0]  == OP_ALL) || arg5.form == 2) free(ptr5);
  if (arg5.form == 2) op_mat_assemble(arg5.dat);
  if ((arg6.form == 1 && arg6.idx[0]  == OP_ALL) || arg6.form == 2) free(ptr6);
  if (arg6.form == 2) op_mat_assemble(arg6.dat);

}

void op_par_loop_8 ( void (*kernel)(void *, void *, void *, void *, void *, void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2,
                     op_arg arg3,
                     op_arg arg4,
                     op_arg arg5,
                     op_arg arg6,
                     op_arg arg7
                   )

{
  arg_check ( set, 0, &arg0 , name );
  arg_check ( set, 1, &arg1 , name );
  arg_check ( set, 2, &arg2 , name );
  arg_check ( set, 3, &arg3 , name );
  arg_check ( set, 4, &arg4 , name );
  arg_check ( set, 5, &arg5 , name );
  arg_check ( set, 6, &arg6 , name );
  arg_check ( set, 7, &arg7 , name );

  char * ptr0 = 0, * ptr1 = 0, * ptr2 = 0, * ptr3 = 0, * ptr4 = 0, * ptr5 = 0, * ptr6 = 0, * ptr7 = 0;

  switch( arg0.form ) {
  case 0:
    ptr0 = (char*) arg0.dat->dat;
    break;
  case 1:
    if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
    break;
  case 2:
    ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.map[1]->dim * arg0.dat->size);
    break;
  }

  switch( arg1.form ) {
  case 0:
    ptr1 = (char*) arg1.dat->dat;
    break;
  case 1:
    if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
    break;
  case 2:
    ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.map[1]->dim * arg1.dat->size);
    break;
  }

  switch( arg2.form ) {
  case 0:
    ptr2 = (char*) arg2.dat->dat;
    break;
  case 1:
    if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);
    break;
  case 2:
    ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.map[1]->dim * arg2.dat->size);
    break;
  }

  switch( arg3.form ) {
  case 0:
    ptr3 = (char*) arg3.dat->dat;
    break;
  case 1:
    if (arg3.idx[0]  == OP_ALL) ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.dat->size);
    break;
  case 2:
    ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.map[1]->dim * arg3.dat->size);
    break;
  }

  switch( arg4.form ) {
  case 0:
    ptr4 = (char*) arg4.dat->dat;
    break;
  case 1:
    if (arg4.idx[0]  == OP_ALL) ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.dat->size);
    break;
  case 2:
    ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.map[1]->dim * arg4.dat->size);
    break;
  }

  switch( arg5.form ) {
  case 0:
    ptr5 = (char*) arg5.dat->dat;
    break;
  case 1:
    if (arg5.idx[0]  == OP_ALL) ptr5 = (char*) malloc(arg5.map[0]->dim * arg5.dat->size);
    break;
  case 2:
    ptr5 = (char*) malloc(arg5.map[0]->dim * arg5.map[1]->dim * arg5.dat->size);
    break;
  }

  switch( arg6.form ) {
  case 0:
    ptr6 = (char*) arg6.dat->dat;
    break;
  case 1:
    if (arg6.idx[0]  == OP_ALL) ptr6 = (char*) malloc(arg6.map[0]->dim * arg6.dat->size);
    break;
  case 2:
    ptr6 = (char*) malloc(arg6.map[0]->dim * arg6.map[1]->dim * arg6.dat->size);
    break;
  }

  switch( arg7.form ) {
  case 0:
    ptr7 = (char*) arg7.dat->dat;
    break;
  case 1:
    if (arg7.idx[0]  == OP_ALL) ptr7 = (char*) malloc(arg7.map[0]->dim * arg7.dat->size);
    break;
  case 2:
    ptr7 = (char*) malloc(arg7.map[0]->dim * arg7.map[1]->dim * arg7.dat->size);
    break;
  }


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.form == 1) {
      if (arg0.idx[0]  == OP_ALL) {
        if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
          copy_in(i, arg0.dat, arg0.map[0], ptr0 );
      } else {
        arg_set ( i, &arg0, &ptr0 );
      }
    }

    if (arg1.form == 1) {
      if (arg1.idx[0]  == OP_ALL) {
        if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
          copy_in(i, arg1.dat, arg1.map[0], ptr1 );
      } else {
        arg_set ( i, &arg1, &ptr1 );
      }
    }

    if (arg2.form == 1) {
      if (arg2.idx[0]  == OP_ALL) {
        if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
          copy_in(i, arg2.dat, arg2.map[0], ptr2 );
      } else {
        arg_set ( i, &arg2, &ptr2 );
      }
    }

    if (arg3.form == 1) {
      if (arg3.idx[0]  == OP_ALL) {
        if (arg3.acc  == OP_READ || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
          copy_in(i, arg3.dat, arg3.map[0], ptr3 );
      } else {
        arg_set ( i, &arg3, &ptr3 );
      }
    }

    if (arg4.form == 1) {
      if (arg4.idx[0]  == OP_ALL) {
        if (arg4.acc  == OP_READ || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
          copy_in(i, arg4.dat, arg4.map[0], ptr4 );
      } else {
        arg_set ( i, &arg4, &ptr4 );
      }
    }

    if (arg5.form == 1) {
      if (arg5.idx[0]  == OP_ALL) {
        if (arg5.acc  == OP_READ || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
          copy_in(i, arg5.dat, arg5.map[0], ptr5 );
      } else {
        arg_set ( i, &arg5, &ptr5 );
      }
    }

    if (arg6.form == 1) {
      if (arg6.idx[0]  == OP_ALL) {
        if (arg6.acc  == OP_READ || arg6.acc  == OP_RW || arg6.acc  == OP_INC)
          copy_in(i, arg6.dat, arg6.map[0], ptr6 );
      } else {
        arg_set ( i, &arg6, &ptr6 );
      }
    }

    if (arg7.form == 1) {
      if (arg7.idx[0]  == OP_ALL) {
        if (arg7.acc  == OP_READ || arg7.acc  == OP_RW || arg7.acc  == OP_INC)
          copy_in(i, arg7.dat, arg7.map[0], ptr7 );
      } else {
        arg_set ( i, &arg7, &ptr7 );
      }
    }

    kernel(ptr0, ptr1, ptr2, ptr3, ptr4, ptr5, ptr6, ptr7);

    if (arg0.form == 1 && arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    } else if (arg0.form == 2) {
      const int rows = arg0.map[0]->dim;
      const int cols = arg0.map[1]->dim;
      op_mat_addto( arg0.dat, ptr0, rows, arg0.map[0]->map + i*rows, cols, arg0.map[1]->map + i*cols);
    }

    if (arg1.form == 1 && arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    } else if (arg1.form == 2) {
      const int rows = arg1.map[0]->dim;
      const int cols = arg1.map[1]->dim;
      op_mat_addto( arg1.dat, ptr1, rows, arg1.map[0]->map + i*rows, cols, arg1.map[1]->map + i*cols);
    }

    if (arg2.form == 1 && arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
    } else if (arg2.form == 2) {
      const int rows = arg2.map[0]->dim;
      const int cols = arg2.map[1]->dim;
      op_mat_addto( arg2.dat, ptr2, rows, arg2.map[0]->map + i*rows, cols, arg2.map[1]->map + i*cols);
    }

    if (arg3.form == 1 && arg3.idx[0]  == OP_ALL) {
      if (arg3.acc  == OP_WRITE || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_out(i, arg3.dat, arg3.map[0], ptr3 );
    } else if (arg3.form == 2) {
      const int rows = arg3.map[0]->dim;
      const int cols = arg3.map[1]->dim;
      op_mat_addto( arg3.dat, ptr3, rows, arg3.map[0]->map + i*rows, cols, arg3.map[1]->map + i*cols);
    }

    if (arg4.form == 1 && arg4.idx[0]  == OP_ALL) {
      if (arg4.acc  == OP_WRITE || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_out(i, arg4.dat, arg4.map[0], ptr4 );
    } else if (arg4.form == 2) {
      const int rows = arg4.map[0]->dim;
      const int cols = arg4.map[1]->dim;
      op_mat_addto( arg4.dat, ptr4, rows, arg4.map[0]->map + i*rows, cols, arg4.map[1]->map + i*cols);
    }

    if (arg5.form == 1 && arg5.idx[0]  == OP_ALL) {
      if (arg5.acc  == OP_WRITE || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
        copy_out(i, arg5.dat, arg5.map[0], ptr5 );
    } else if (arg5.form == 2) {
      const int rows = arg5.map[0]->dim;
      const int cols = arg5.map[1]->dim;
      op_mat_addto( arg5.dat, ptr5, rows, arg5.map[0]->map + i*rows, cols, arg5.map[1]->map + i*cols);
    }

    if (arg6.form == 1 && arg6.idx[0]  == OP_ALL) {
      if (arg6.acc  == OP_WRITE || arg6.acc  == OP_RW || arg6.acc  == OP_INC)
        copy_out(i, arg6.dat, arg6.map[0], ptr6 );
    } else if (arg6.form == 2) {
      const int rows = arg6.map[0]->dim;
      const int cols = arg6.map[1]->dim;
      op_mat_addto( arg6.dat, ptr6, rows, arg6.map[0]->map + i*rows, cols, arg6.map[1]->map + i*cols);
    }

    if (arg7.form == 1 && arg7.idx[0]  == OP_ALL) {
      if (arg7.acc  == OP_WRITE || arg7.acc  == OP_RW || arg7.acc  == OP_INC)
        copy_out(i, arg7.dat, arg7.map[0], ptr7 );
    } else if (arg7.form == 2) {
      const int rows = arg7.map[0]->dim;
      const int cols = arg7.map[1]->dim;
      op_mat_addto( arg7.dat, ptr7, rows, arg7.map[0]->map + i*rows, cols, arg7.map[1]->map + i*cols);
    }

  }

  if ((arg0.form == 1 && arg0.idx[0]  == OP_ALL) || arg0.form == 2) free(ptr0);
  if (arg0.form == 2) op_mat_assemble(arg0.dat);
  if ((arg1.form == 1 && arg1.idx[0]  == OP_ALL) || arg1.form == 2) free(ptr1);
  if (arg1.form == 2) op_mat_assemble(arg1.dat);
  if ((arg2.form == 1 && arg2.idx[0]  == OP_ALL) || arg2.form == 2) free(ptr2);
  if (arg2.form == 2) op_mat_assemble(arg2.dat);
  if ((arg3.form == 1 && arg3.idx[0]  == OP_ALL) || arg3.form == 2) free(ptr3);
  if (arg3.form == 2) op_mat_assemble(arg3.dat);
  if ((arg4.form == 1 && arg4.idx[0]  == OP_ALL) || arg4.form == 2) free(ptr4);
  if (arg4.form == 2) op_mat_assemble(arg4.dat);
  if ((arg5.form == 1 && arg5.idx[0]  == OP_ALL) || arg5.form == 2) free(ptr5);
  if (arg5.form == 2) op_mat_assemble(arg5.dat);
  if ((arg6.form == 1 && arg6.idx[0]  == OP_ALL) || arg6.form == 2) free(ptr6);
  if (arg6.form == 2) op_mat_assemble(arg6.dat);
  if ((arg7.form == 1 && arg7.idx[0]  == OP_ALL) || arg7.form == 2) free(ptr7);
  if (arg7.form == 2) op_mat_assemble(arg7.dat);

}

