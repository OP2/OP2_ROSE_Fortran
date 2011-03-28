#include "op_datatypes.h"

void initialise_set ( op_set * set, int size, int index, char const * name ){
  set->size = size;
  set->index = index;
  set->name = name;
}

void initialise_map ( op_map * mapping, op_set from, op_set to, int dim, int index, int * map, char const * name ){
  mapping->from = from;
  mapping->to   = to;
  mapping->dim  = dim;
  mapping->index = index;
  mapping->map  = map;
  mapping->name = name;
}

void initialise_dat ( op_dat * data, op_set set, int dim, int index, int size, void *dat, char const * name ){
  data->set   = set;
  data->dim   = dim;
  data->index = index;
  data->dat   = dat;
  data->size  = dim*size;
	data->name = name;
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

