#include <stdio.h>

#include "op_common.h"

//
// global variables
//

int OP_set_index=0, OP_set_max=0,
    OP_map_index=0, OP_map_max=0,
    OP_dat_index=0, OP_dat_max=0,
    OP_diags    =0;

op_set  **OP_set_list;
op_map  **OP_map_list;
op_dat  **OP_dat_list;

/*
 * OP functions
 */

void op_init(int argc, char **argv, int diags)
{
  OP_diags = diags;
}

void op_fetch_data(op_dat dat) {}

void op_diagnostic_output(){
  if (OP_diags > 1) {
    printf("\n  OP diagnostic output\n");
    printf(  "  --------------------\n");

    printf("\n       set       size\n");
    printf(  "  -------------------\n");
    for(int n=0; n<OP_set_index; n++) {
      op_set set=*OP_set_list[n];
      printf("%10s %10d\n",set.name,set.size);
    }

    printf("\n       map        dim       from         to\n");
    printf(  "  -----------------------------------------\n");
    for(int n=0; n<OP_map_index; n++) {
      op_map map=*OP_map_list[n];
      printf("%10s %10d %10s %10s\n",map.name,map.dim,map.from.name,map.to.name);
    }

    printf("\n       dat        dim        set\n");
    printf(  "  ------------------------------\n");
    for(int n=0; n<OP_dat_index; n++) {
      op_dat dat=*OP_dat_list[n];
      printf("%10s %10d %10s\n",dat.name,dat.dim,dat.set.name);
    }
    printf("\n");
  }
}

void op_timing_output() {}

void op_exit() {}

