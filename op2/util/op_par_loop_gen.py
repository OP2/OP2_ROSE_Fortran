#! /usr/bin/env python

import sys

maxargs = 8
if len(sys.argv) > 1:
  maxargs = int(sys.argv[1])

file_c = "../reference/op_par_loop_ref.c"
file_h = "../common/op_par_loop.h"

header_h = """/*
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

#ifndef __OP_PAR_LOOP_H
#define __OP_PAR_LOOP_H

#include "op_datatypes.h"

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

header_c = """/*
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

static inline void arg_set (int displacement, op_arg * argument, char ** p_arg)
{
  int n2;

  if ( argument->map[0] == 0 )			// identity mapping
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

# Loop over arguments

    for numargs in range(1,maxargs+1):
      # build op_par_loop signature
      par_loop_sig  = "void op_par_loop_%d ( void (*kernel)(void *" % numargs + (numargs-1) * ", void *" + "), char const * name, op_set * set,\n"
      par_loop_sig += ",\n".join(["                     op_arg arg%d" % (i) for i in range(numargs)])
      par_loop_sig += "\n                   );\n"

      par_loop_body = """
{
"""
      # build argument check calls
      for i in range(numargs):
        par_loop_body += "  arg_check ( set, %d, &arg%d , name );\n" % (i,i)

      # build kernel parameter declarations
      par_loop_body += "\n  char " + ", ".join(["* ptr%d = 0" % i for i in range(numargs)]) + ";\n"

      # build scratch memory allocation
      for i in range(numargs):
        par_loop_body += """
  switch( arg%d.form ) {
  case 0:
    ptr%d = (char*) arg%d.dat->dat;
    break;
  case 1:
    if (arg%d.idx[0]  == OP_ALL) ptr%d = (char*) malloc(arg%d.map[0]->dim * arg%d.dat->size);
    break;
  case 2:
    ptr%d = (char*) malloc(arg%d.map[0]->dim * arg%d.map[1]->dim * arg%d.dat->size);
    break;
  }
""" % tuple(11*[i])

      # build loop over set elements
      par_loop_body += """

  // Loop over set elements
  for ( int i = 0; i < set->size; i++ ) {
"""

      # build copy-in / setting of pointer
      for i in range(numargs):
        par_loop_body += """
    if (arg%d.form == 1) {
      if (arg%d.idx[0]  == OP_ALL) {
        if (arg%d.acc  == OP_READ || arg%d.acc  == OP_RW || arg%d.acc  == OP_INC)
          copy_in(i, arg%d.dat, arg%d.map[0], ptr%d );
      } else {
        arg_set ( i, &arg%d, &ptr%d );
      }
    }
""" % tuple(10*[i])

      # build kernel call
      par_loop_body += "\n    kernel(" + ", ".join(["ptr%d" % i for i in range(numargs)]) + ");\n"

      # build copy-out / matrix assembly call
      for i in range(numargs):
        par_loop_body += """
    if (arg%d.form == 1 && arg%d.idx[0]  == OP_ALL) {
      if (arg%d.acc  == OP_WRITE || arg%d.acc  == OP_RW || arg%d.acc  == OP_INC)
        copy_out(i, arg%d.dat, arg%d.map[0], ptr%d );
    } else if (arg%d.form == 2) {
      const int rows = arg%d.map[0]->dim;
      const int cols = arg%d.map[1]->dim;
      op_mat_addto( arg%d.dat->dat, ptr%d, rows, arg%d.map[0]->map + i*rows, cols, arg%d.map[1]->map + i*cols);
    }
""" % tuple(15*[i])

      par_loop_body += """
  }

"""
      # build scratch memory frees
      for i in range(numargs):
        par_loop_body += "  if ((arg%d.form == 1 && arg%d.idx[0]  == OP_ALL) || arg%d.form == 2) free(ptr%d);\n" % (i,i,i,i)
        par_loop_body += "  if (arg%d.form == 2) op_mat_assemble(arg%d.dat->dat);\n" % (i,i)

      par_loop_body += """
}
"""

      par_loop_func = par_loop_sig[:-2] + "\n" + par_loop_body;

      h.write(par_loop_sig + "\n")
      c.write(par_loop_func + "\n")

# End loop over arguments

    h.write(footer_h)
    c.write(footer_c)

