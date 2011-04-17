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

#include <stdio.h>

#include <petscsys.h>

#include <op2/common/op_globals.h>
#include <op2/common/op_common.h>

/*
 * OP functions
 */

void op_init(int argc, char **argv, int diags)
{
  OP_diags = diags;
  PetscInitialize(&argc,&argv,(char *)0,(char *)0);
}

void op_fetch_data(op_dat dat) {}

void op_diagnostic_output(){
  if (OP_diags > 1) {
    printf("\n  OP diagnostic output\n");
    printf(  "  --------------------\n");

    printf("\n       set       size\n");
    printf(  "  -------------------\n");
    for(int n=0; n<OP_set_index; n++) {
      op_set * set=OP_set_list[n];
      printf("%10s %10d\n",set->name,set->size);
    }

    printf("\n       map        dim       from         to\n");
    printf(  "  -----------------------------------------\n");
    for(int n=0; n<OP_map_index; n++) {
      op_map * map=OP_map_list[n];
      printf("%10s %10d %10s %10s\n",map->name,map->dim,map->from->name,map->to->name);
    }

    printf("\n       dat        dim        set        set\n");
    printf(  "  -----------------------------------------\n");
    for(int n=0; n<OP_dat_index; n++) {
      op_dat * dat=OP_dat_list[n];
      printf("%10s %10d %10s %10s\n",dat->name,dat->dim,dat->rank > 0 ? dat->set[0]->name : "-",dat->rank > 1 ? dat->set[1]->name : "-");
    }
    printf("\n");
  }
}

void op_timing_output() {}

void op_exit() {
  PetscFinalize();
}

