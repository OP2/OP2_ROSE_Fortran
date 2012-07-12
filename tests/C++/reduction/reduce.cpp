#include <stdio.h>
#include <stdlib.h>
#include "op_lib_cpp.h"
#include "count.h"
#include "addto.h"

int main(int argc, char **argv)
{

  int n = 2;
  int n2 = 2;
  op_init(argc, argv, 5);
  op_set set = op_decl_set(n, "set");

  op_set set2 = op_decl_set(n2, "set");

  int *map_d;
  map_d = (int *)malloc(2 * sizeof(int));
  map_d[0] = 1;
  map_d[1] = 0;
  op_map map = op_decl_map(set, set2, 1, map_d, "map");

  int *dat_d;
  dat_d = (int *)malloc(2 * sizeof(int));
  dat_d[0] = 1;
  dat_d[1] = 2;
  op_dat dat = op_decl_dat(set2, 1, "int", dat_d, "dat");

  int count_d = 0;


  op_par_loop(count, "count", set,
              op_arg_gbl(&count_d, 1, "int", OP_INC));

  printf("Count is %d should be %d\n", count_d, 2);

  count_d = 0;
  op_par_loop(addto, "addto", set,
              op_arg_gbl(&count_d, 1, "int", OP_INC),
              op_arg_dat(dat, 0, map, 1, "int", OP_READ));

  printf("Count is %d should be %d\n", count_d, 3);

  op_exit();
}
