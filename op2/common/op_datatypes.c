#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <op2/common/op_datatypes.h>

/*
 * Global variables
 */

int OP_set_index=0, OP_set_max=0,
    OP_map_index=0, OP_map_max=0,
    OP_dat_index=0, OP_dat_max=0,
    OP_diags    =0;

op_set  **OP_set_list;
op_map  **OP_map_list;
op_dat  **OP_dat_list;

void initialise_set ( op_set * set, int size, char const * name )
{
  set->size = size;
  set->index = OP_set_index;
  set->name = name;

  if (OP_set_index==OP_set_max) {
    OP_set_max += 10;
    OP_set_list = (op_set **) realloc(OP_set_list,OP_set_max*sizeof(op_set *));
    if (OP_set_list==NULL) {
      printf(" op_decl_set error -- error reallocating memory\n");
      exit(-1);
    }
  }

  OP_set_list[OP_set_index++] = set;
}

void initialise_map ( op_map * mapping, op_set * from, op_set * to, int dim, int * map, char const * name )
{

  if ( (from->index<0) || (from->index>=OP_set_index) ||
       strcmp(OP_set_list[from->index]->name,from->name) ) {
    printf(" op_decl_map error -- invalid 'from' set for map %s\n",name);
    exit(-1);
  }

  if ( (to->index<0) || (to->index>=OP_set_index) ||
       strcmp(OP_set_list[to->index]->name,to->name) ) {
    printf("op_decl_map error -- invalid 'to' set for map %s\n",name);
    exit(-1);
  }

  if (dim<=0) {
    printf("op_decl_map error -- negative/zero dimension for map %s\n",name);
    exit(-1);
  }

  for (int d=0; d<dim; d++) {
    for (int n=0; n<from->size; n++) {
      if (map[d+n*dim]<0 || map[d+n*dim]>=to->size) {
        printf("op_decl_map error -- invalid data for map %s\n",name);
        printf("element = %d, dimension = %d, map = %d\n",n,d,map[d+n*dim]);
        exit(-1);
      }
    }
  }

  mapping->from = *from;
  mapping->to   = *to;
  mapping->dim  = dim;
  mapping->index = OP_map_index;
  mapping->map  = map;
  mapping->name = name;

  if (OP_map_index==OP_map_max) {
    OP_map_max += 10;
    OP_map_list = (op_map **) realloc(OP_map_list,OP_map_max*sizeof(op_map *));
    if (OP_map_list==NULL) {
      printf(" op_decl_map error -- error reallocating memory\n");
      exit(-1);
    }
  }

  OP_map_list[OP_map_index++] = mapping;
}

void initialise_dat ( op_dat * data, op_set * set, int dim, int size, void *dat, char const * name )
{

  if ( set && (set->index<0 || set->index>=OP_set_index ||
       strcmp(OP_set_list[set->index]->name,set->name)) ) {
    printf("op_decl_dat error -- invalid set for data: %s\n",name);
    exit(-1);
  }

  if (dim<=0) {
    printf("op_decl_dat error -- negative/zero dimension for data: %s\n",name);
    exit(-1);
  }

  data->set   = set ? *set : OP_NULL_SET;
  data->dim   = dim;
  data->index = OP_dat_index;
  data->dat   = dat;
  data->size  = dim*size;
	data->name = name;

  if (OP_dat_index==OP_dat_max) {
    OP_dat_max += 10;
    OP_dat_list = (op_dat **) realloc(OP_dat_list,OP_dat_max*sizeof(op_dat *));
    if (OP_dat_list==NULL) {
      printf(" op_decl_dat error -- error reallocating memory\n");
      exit(-1);
    }
  }

  OP_dat_list[OP_dat_index++] = data;
}

void op_decl_id_map ( op_map * map )
{
	op_set nullSet = {0,0,"null"};

	map->from = nullSet;
	map->to = nullSet;
	map->dim = 0; //set to the proper value in Fortran
	map->index= -1; // no position in OP_map_list
	map->map = 0;
	map->name = "id";
}

void op_decl_gbl_map ( op_map * map )
{
	op_set nullSet = {0,0,"null"};

	map->from = nullSet;
	map->to = nullSet;
	map->dim = 0; //set to the proper value in Fortran
	map->index= -2; // no position in OP_map_list
	map->map = 0;
	map->name = "gbl";
}

op_arg op_construct_gbl_arg(op_dat * data, op_access acc)
{
  op_arg arg = (op_arg) {
    0,      /*arg.form = 0;*/
    data,   /*arg.dat = data;*/
    {0, 0}, /*arg.idx = { 0, 0 };*/
    {0, 0}, /*arg.map = { 0, 0 };*/
    acc     /*arg.acc = acc;*/
  };

  return arg;
}

op_arg op_construct_vec_arg(op_dat * data, int idx, op_map * map, op_access acc)
{
  op_arg arg = (op_arg) {
    1,          /*arg.form = 1;*/
    data,       /*arg.dat = data;*/
    { idx, 0 }, /*arg.idx = { idx, 0 };*/
    { map, 0 }, /*arg.map = { map, 0 };*/
    acc         /*arg.acc = acc;*/
  };

  return arg;
}

op_arg op_construct_mat_arg(op_dat * data, int idx0, op_map * map0, int idx1, op_map * map1, op_access acc)
{
  op_arg arg = (op_arg) {
    2,              /*arg.form = 2;*/
    data,           /*arg.dat = data;*/
    { idx0, idx1 }, /*arg.idx = { idx0, idx1 };*/
    { map0, map1 }, /*arg.map = { map0, map1 };*/
    acc             /*arg.acc = acc;*/
  };

  return arg;
}

