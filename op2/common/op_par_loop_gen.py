#! /usr/bin/env python

import sys

maxargs = 8
if len(sys.argv) > 1:
  maxargs = int(sys.argv[1])

filename = "op_par_loop"
if len(sys.argv) > 2:
  filename = sys.argv[2]

file_c = filename + ".c"
file_h = filename + ".h"

header_h = """
#ifndef __OP_PAR_LOOP_H
#define __OP_PAR_LOOP_H

#include <op2/common/op_datatypes.h>

#ifdef __cplusplus
extern "C" {
#endif

"""

footer_h = """

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
"""

header_c = """
/*
 * header files
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "op_par_loop.h"

static inline void arg_check(op_set * set, int argnum, op_arg * argument, const char * kernel) {

  for (int i = 0; i < argument->form; ++i) {
    if (argument->map[i] == NULL) {
      if (argument->idx[i] != OP_NONE) {
        printf("error: arg %d in kernel %s\\n", argnum, kernel);
        printf("invalid index, should be OP_NONE for identity mapping %d\\n", i);
        exit(1);
      }
    }
    else {
      if (set->index != argument->map[i]->from.index
          || argument->dat->set.index != argument->map[i]->to.index) {
        printf("error: arg %d in kernel %s\\n", argnum, kernel);
        printf("invalid mapping %d\\n", i);
        exit(1);
      }
      if (argument->idx[i] != OP_ALL && (argument->idx[i] < 0 || argument->idx[i] >= argument->map[i]->dim)) {
        printf("error: arg %d in kernel %s\\n", argnum, kernel);
        printf("invalid index into mapping %d\\n", i);
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
    printf("error: arg_set failed for set element %d (don't know what to do)\\n", displacement);

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

"""

footer_c = ""

with open(file_h,"w") as h:
  with open(file_c,"w") as c:

    h.write(header_h)
    c.write(header_c)

    for numargs in range(1,maxargs+1):
      par_loop_sig  = "void op_par_loop_%d ( void (*kernel)(void *" % numargs + (numargs-1) * ", void *" + "), char const * name, op_set * set,\n"
      par_loop_sig += ",\n".join(["                     op_arg arg%d" % (i) for i in range(numargs)])
      par_loop_sig += "\n                   );\n"

      par_loop_body = """
{
"""
      for i in range(numargs):
        par_loop_body += "  arg_check ( set, %d, &arg%d , name );\n" % (i,i)

      par_loop_body += "\n  char " + ", ".join(["* ptr%d = 0" % i for i in range(numargs)]) + ";\n\n"

      par_loop_body += "  // Allocate memory for copy-in\n"
      for i in range(numargs):
        par_loop_body += "  if (arg%d.idx[0]  == OP_ALL) ptr%d = (char*) malloc(arg%d.map[0]->dim * arg%d.dat->size);\n" % (i,i,i,i)

      par_loop_body += """

  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {
"""

      for i in range(numargs):
        par_loop_body += """
    if (arg%d.idx[0]  == OP_ALL) {
      if (arg%d.acc  == OP_READ || arg%d.acc  == OP_RW || arg%d.acc  == OP_INC)
        copy_in(i, arg%d.dat, arg%d.map[0], ptr%d );
    } else {
      arg_set ( i, &arg%d, &ptr%d );
    }""" % tuple(9*[i])

      par_loop_body += "\n    kernel(" + ", ".join(["ptr%d" % i for i in range(numargs)]) + ");\n"

      for i in range(numargs):
        par_loop_body += """
    if (arg%d.idx[0]  == OP_ALL)
      if (arg%d.acc  == OP_WRITE || arg%d.acc  == OP_RW || arg%d.acc  == OP_INC)
        copy_out(i, arg%d.dat, arg%d.map[0], ptr%d );""" % tuple(7*[i])

      par_loop_body += """
  }
}
"""

      par_loop_func = par_loop_sig[:-2] + "\n" + par_loop_body;

      h.write(par_loop_sig + "\n")
      c.write(par_loop_func + "\n")

    h.write(footer_h)
    c.write(footer_c)

