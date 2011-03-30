
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
      if (set->index != argument->map[i]->from.index
          || argument->dat->set.index != argument->map[i]->to.index) {
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

// FIXME: what to do with matrix (2-form) arguments???
static inline void arg_set (int displacement, op_arg * argument, char ** p_arg)
{
  int n2;

  if ( argument->form == 0 )					      // global variable, no mapping at all
		n2 = 0;
  else if ( argument->map[0] == 0 )			// identity mapping
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

  // Allocate memory for copy-in
  if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_in(i, arg0.dat, arg0.map[0], ptr0 );
    } else {
      arg_set ( i, &arg0, &ptr0 );
    }
    kernel(ptr0);

    if (arg0.idx[0]  == OP_ALL)
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
  }
}

void op_par_loop_2 ( void (*kernel)(void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1
                   )

{
  arg_check ( set, 0, &arg0 , name );
  arg_check ( set, 1, &arg1 , name );

  char * ptr0 = 0, * ptr1 = 0;

  // Allocate memory for copy-in
  if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
  if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_in(i, arg0.dat, arg0.map[0], ptr0 );
    } else {
      arg_set ( i, &arg0, &ptr0 );
    }
    if (arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_in(i, arg1.dat, arg1.map[0], ptr1 );
    } else {
      arg_set ( i, &arg1, &ptr1 );
    }
    kernel(ptr0, ptr1);

    if (arg0.idx[0]  == OP_ALL)
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    if (arg1.idx[0]  == OP_ALL)
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
  }
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

  // Allocate memory for copy-in
  if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
  if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
  if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_in(i, arg0.dat, arg0.map[0], ptr0 );
    } else {
      arg_set ( i, &arg0, &ptr0 );
    }
    if (arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_in(i, arg1.dat, arg1.map[0], ptr1 );
    } else {
      arg_set ( i, &arg1, &ptr1 );
    }
    if (arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_in(i, arg2.dat, arg2.map[0], ptr2 );
    } else {
      arg_set ( i, &arg2, &ptr2 );
    }
    kernel(ptr0, ptr1, ptr2);

    if (arg0.idx[0]  == OP_ALL)
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    if (arg1.idx[0]  == OP_ALL)
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    if (arg2.idx[0]  == OP_ALL)
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
  }
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

  // Allocate memory for copy-in
  if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
  if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
  if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);
  if (arg3.idx[0]  == OP_ALL) ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.dat->size);


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_in(i, arg0.dat, arg0.map[0], ptr0 );
    } else {
      arg_set ( i, &arg0, &ptr0 );
    }
    if (arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_in(i, arg1.dat, arg1.map[0], ptr1 );
    } else {
      arg_set ( i, &arg1, &ptr1 );
    }
    if (arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_in(i, arg2.dat, arg2.map[0], ptr2 );
    } else {
      arg_set ( i, &arg2, &ptr2 );
    }
    if (arg3.idx[0]  == OP_ALL) {
      if (arg3.acc  == OP_READ || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_in(i, arg3.dat, arg3.map[0], ptr3 );
    } else {
      arg_set ( i, &arg3, &ptr3 );
    }
    kernel(ptr0, ptr1, ptr2, ptr3);

    if (arg0.idx[0]  == OP_ALL)
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    if (arg1.idx[0]  == OP_ALL)
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    if (arg2.idx[0]  == OP_ALL)
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
    if (arg3.idx[0]  == OP_ALL)
      if (arg3.acc  == OP_WRITE || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_out(i, arg3.dat, arg3.map[0], ptr3 );
  }
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

  // Allocate memory for copy-in
  if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
  if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
  if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);
  if (arg3.idx[0]  == OP_ALL) ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.dat->size);
  if (arg4.idx[0]  == OP_ALL) ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.dat->size);


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_in(i, arg0.dat, arg0.map[0], ptr0 );
    } else {
      arg_set ( i, &arg0, &ptr0 );
    }
    if (arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_in(i, arg1.dat, arg1.map[0], ptr1 );
    } else {
      arg_set ( i, &arg1, &ptr1 );
    }
    if (arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_in(i, arg2.dat, arg2.map[0], ptr2 );
    } else {
      arg_set ( i, &arg2, &ptr2 );
    }
    if (arg3.idx[0]  == OP_ALL) {
      if (arg3.acc  == OP_READ || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_in(i, arg3.dat, arg3.map[0], ptr3 );
    } else {
      arg_set ( i, &arg3, &ptr3 );
    }
    if (arg4.idx[0]  == OP_ALL) {
      if (arg4.acc  == OP_READ || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_in(i, arg4.dat, arg4.map[0], ptr4 );
    } else {
      arg_set ( i, &arg4, &ptr4 );
    }
    kernel(ptr0, ptr1, ptr2, ptr3, ptr4);

    if (arg0.idx[0]  == OP_ALL)
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    if (arg1.idx[0]  == OP_ALL)
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    if (arg2.idx[0]  == OP_ALL)
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
    if (arg3.idx[0]  == OP_ALL)
      if (arg3.acc  == OP_WRITE || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_out(i, arg3.dat, arg3.map[0], ptr3 );
    if (arg4.idx[0]  == OP_ALL)
      if (arg4.acc  == OP_WRITE || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_out(i, arg4.dat, arg4.map[0], ptr4 );
  }
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

  // Allocate memory for copy-in
  if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
  if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
  if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);
  if (arg3.idx[0]  == OP_ALL) ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.dat->size);
  if (arg4.idx[0]  == OP_ALL) ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.dat->size);
  if (arg5.idx[0]  == OP_ALL) ptr5 = (char*) malloc(arg5.map[0]->dim * arg5.dat->size);


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_in(i, arg0.dat, arg0.map[0], ptr0 );
    } else {
      arg_set ( i, &arg0, &ptr0 );
    }
    if (arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_in(i, arg1.dat, arg1.map[0], ptr1 );
    } else {
      arg_set ( i, &arg1, &ptr1 );
    }
    if (arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_in(i, arg2.dat, arg2.map[0], ptr2 );
    } else {
      arg_set ( i, &arg2, &ptr2 );
    }
    if (arg3.idx[0]  == OP_ALL) {
      if (arg3.acc  == OP_READ || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_in(i, arg3.dat, arg3.map[0], ptr3 );
    } else {
      arg_set ( i, &arg3, &ptr3 );
    }
    if (arg4.idx[0]  == OP_ALL) {
      if (arg4.acc  == OP_READ || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_in(i, arg4.dat, arg4.map[0], ptr4 );
    } else {
      arg_set ( i, &arg4, &ptr4 );
    }
    if (arg5.idx[0]  == OP_ALL) {
      if (arg5.acc  == OP_READ || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
        copy_in(i, arg5.dat, arg5.map[0], ptr5 );
    } else {
      arg_set ( i, &arg5, &ptr5 );
    }
    kernel(ptr0, ptr1, ptr2, ptr3, ptr4, ptr5);

    if (arg0.idx[0]  == OP_ALL)
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    if (arg1.idx[0]  == OP_ALL)
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    if (arg2.idx[0]  == OP_ALL)
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
    if (arg3.idx[0]  == OP_ALL)
      if (arg3.acc  == OP_WRITE || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_out(i, arg3.dat, arg3.map[0], ptr3 );
    if (arg4.idx[0]  == OP_ALL)
      if (arg4.acc  == OP_WRITE || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_out(i, arg4.dat, arg4.map[0], ptr4 );
    if (arg5.idx[0]  == OP_ALL)
      if (arg5.acc  == OP_WRITE || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
        copy_out(i, arg5.dat, arg5.map[0], ptr5 );
  }
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

  // Allocate memory for copy-in
  if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
  if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
  if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);
  if (arg3.idx[0]  == OP_ALL) ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.dat->size);
  if (arg4.idx[0]  == OP_ALL) ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.dat->size);
  if (arg5.idx[0]  == OP_ALL) ptr5 = (char*) malloc(arg5.map[0]->dim * arg5.dat->size);
  if (arg6.idx[0]  == OP_ALL) ptr6 = (char*) malloc(arg6.map[0]->dim * arg6.dat->size);


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_in(i, arg0.dat, arg0.map[0], ptr0 );
    } else {
      arg_set ( i, &arg0, &ptr0 );
    }
    if (arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_in(i, arg1.dat, arg1.map[0], ptr1 );
    } else {
      arg_set ( i, &arg1, &ptr1 );
    }
    if (arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_in(i, arg2.dat, arg2.map[0], ptr2 );
    } else {
      arg_set ( i, &arg2, &ptr2 );
    }
    if (arg3.idx[0]  == OP_ALL) {
      if (arg3.acc  == OP_READ || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_in(i, arg3.dat, arg3.map[0], ptr3 );
    } else {
      arg_set ( i, &arg3, &ptr3 );
    }
    if (arg4.idx[0]  == OP_ALL) {
      if (arg4.acc  == OP_READ || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_in(i, arg4.dat, arg4.map[0], ptr4 );
    } else {
      arg_set ( i, &arg4, &ptr4 );
    }
    if (arg5.idx[0]  == OP_ALL) {
      if (arg5.acc  == OP_READ || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
        copy_in(i, arg5.dat, arg5.map[0], ptr5 );
    } else {
      arg_set ( i, &arg5, &ptr5 );
    }
    if (arg6.idx[0]  == OP_ALL) {
      if (arg6.acc  == OP_READ || arg6.acc  == OP_RW || arg6.acc  == OP_INC)
        copy_in(i, arg6.dat, arg6.map[0], ptr6 );
    } else {
      arg_set ( i, &arg6, &ptr6 );
    }
    kernel(ptr0, ptr1, ptr2, ptr3, ptr4, ptr5, ptr6);

    if (arg0.idx[0]  == OP_ALL)
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    if (arg1.idx[0]  == OP_ALL)
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    if (arg2.idx[0]  == OP_ALL)
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
    if (arg3.idx[0]  == OP_ALL)
      if (arg3.acc  == OP_WRITE || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_out(i, arg3.dat, arg3.map[0], ptr3 );
    if (arg4.idx[0]  == OP_ALL)
      if (arg4.acc  == OP_WRITE || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_out(i, arg4.dat, arg4.map[0], ptr4 );
    if (arg5.idx[0]  == OP_ALL)
      if (arg5.acc  == OP_WRITE || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
        copy_out(i, arg5.dat, arg5.map[0], ptr5 );
    if (arg6.idx[0]  == OP_ALL)
      if (arg6.acc  == OP_WRITE || arg6.acc  == OP_RW || arg6.acc  == OP_INC)
        copy_out(i, arg6.dat, arg6.map[0], ptr6 );
  }
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

  // Allocate memory for copy-in
  if (arg0.idx[0]  == OP_ALL) ptr0 = (char*) malloc(arg0.map[0]->dim * arg0.dat->size);
  if (arg1.idx[0]  == OP_ALL) ptr1 = (char*) malloc(arg1.map[0]->dim * arg1.dat->size);
  if (arg2.idx[0]  == OP_ALL) ptr2 = (char*) malloc(arg2.map[0]->dim * arg2.dat->size);
  if (arg3.idx[0]  == OP_ALL) ptr3 = (char*) malloc(arg3.map[0]->dim * arg3.dat->size);
  if (arg4.idx[0]  == OP_ALL) ptr4 = (char*) malloc(arg4.map[0]->dim * arg4.dat->size);
  if (arg5.idx[0]  == OP_ALL) ptr5 = (char*) malloc(arg5.map[0]->dim * arg5.dat->size);
  if (arg6.idx[0]  == OP_ALL) ptr6 = (char*) malloc(arg6.map[0]->dim * arg6.dat->size);
  if (arg7.idx[0]  == OP_ALL) ptr7 = (char*) malloc(arg7.map[0]->dim * arg7.dat->size);


  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {

    if (arg0.idx[0]  == OP_ALL) {
      if (arg0.acc  == OP_READ || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_in(i, arg0.dat, arg0.map[0], ptr0 );
    } else {
      arg_set ( i, &arg0, &ptr0 );
    }
    if (arg1.idx[0]  == OP_ALL) {
      if (arg1.acc  == OP_READ || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_in(i, arg1.dat, arg1.map[0], ptr1 );
    } else {
      arg_set ( i, &arg1, &ptr1 );
    }
    if (arg2.idx[0]  == OP_ALL) {
      if (arg2.acc  == OP_READ || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_in(i, arg2.dat, arg2.map[0], ptr2 );
    } else {
      arg_set ( i, &arg2, &ptr2 );
    }
    if (arg3.idx[0]  == OP_ALL) {
      if (arg3.acc  == OP_READ || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_in(i, arg3.dat, arg3.map[0], ptr3 );
    } else {
      arg_set ( i, &arg3, &ptr3 );
    }
    if (arg4.idx[0]  == OP_ALL) {
      if (arg4.acc  == OP_READ || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_in(i, arg4.dat, arg4.map[0], ptr4 );
    } else {
      arg_set ( i, &arg4, &ptr4 );
    }
    if (arg5.idx[0]  == OP_ALL) {
      if (arg5.acc  == OP_READ || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
        copy_in(i, arg5.dat, arg5.map[0], ptr5 );
    } else {
      arg_set ( i, &arg5, &ptr5 );
    }
    if (arg6.idx[0]  == OP_ALL) {
      if (arg6.acc  == OP_READ || arg6.acc  == OP_RW || arg6.acc  == OP_INC)
        copy_in(i, arg6.dat, arg6.map[0], ptr6 );
    } else {
      arg_set ( i, &arg6, &ptr6 );
    }
    if (arg7.idx[0]  == OP_ALL) {
      if (arg7.acc  == OP_READ || arg7.acc  == OP_RW || arg7.acc  == OP_INC)
        copy_in(i, arg7.dat, arg7.map[0], ptr7 );
    } else {
      arg_set ( i, &arg7, &ptr7 );
    }
    kernel(ptr0, ptr1, ptr2, ptr3, ptr4, ptr5, ptr6, ptr7);

    if (arg0.idx[0]  == OP_ALL)
      if (arg0.acc  == OP_WRITE || arg0.acc  == OP_RW || arg0.acc  == OP_INC)
        copy_out(i, arg0.dat, arg0.map[0], ptr0 );
    if (arg1.idx[0]  == OP_ALL)
      if (arg1.acc  == OP_WRITE || arg1.acc  == OP_RW || arg1.acc  == OP_INC)
        copy_out(i, arg1.dat, arg1.map[0], ptr1 );
    if (arg2.idx[0]  == OP_ALL)
      if (arg2.acc  == OP_WRITE || arg2.acc  == OP_RW || arg2.acc  == OP_INC)
        copy_out(i, arg2.dat, arg2.map[0], ptr2 );
    if (arg3.idx[0]  == OP_ALL)
      if (arg3.acc  == OP_WRITE || arg3.acc  == OP_RW || arg3.acc  == OP_INC)
        copy_out(i, arg3.dat, arg3.map[0], ptr3 );
    if (arg4.idx[0]  == OP_ALL)
      if (arg4.acc  == OP_WRITE || arg4.acc  == OP_RW || arg4.acc  == OP_INC)
        copy_out(i, arg4.dat, arg4.map[0], ptr4 );
    if (arg5.idx[0]  == OP_ALL)
      if (arg5.acc  == OP_WRITE || arg5.acc  == OP_RW || arg5.acc  == OP_INC)
        copy_out(i, arg5.dat, arg5.map[0], ptr5 );
    if (arg6.idx[0]  == OP_ALL)
      if (arg6.acc  == OP_WRITE || arg6.acc  == OP_RW || arg6.acc  == OP_INC)
        copy_out(i, arg6.dat, arg6.map[0], ptr6 );
    if (arg7.idx[0]  == OP_ALL)
      if (arg7.acc  == OP_WRITE || arg7.acc  == OP_RW || arg7.acc  == OP_INC)
        copy_out(i, arg7.dat, arg7.map[0], ptr7 );
  }
}

