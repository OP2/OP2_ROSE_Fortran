#include "op_globals.h"
#include "op_datatypes.h"

void op_decl_set ( op_set * set, int size, char const * name ){
  initialise_set ( set, size, OP_set_index, name );
  OP_set_list[OP_set_index++] = set;
}

void op_decl_map ( op_map * mapping, op_set * from, op_set * to, int dim, int * map, char const * name ){
  initialise_map ( mapping, from, to, dim, OP_map_index, map, name );
  OP_map_list[OP_map_index++] = mapping;
}

void op_decl_dat ( op_dat * data, op_set * set, int dim, int size, void *dat, char const * name ){
  initialise_dat ( data, set, dim, OP_dat_index, size, dat, name );
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

