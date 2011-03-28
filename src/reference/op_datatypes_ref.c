#include "op_datatypes.h"

void op_decl_set ( op_set * set, int size, char const * name )
{
  initialise_set ( set, size, name );
}

void op_decl_map ( op_map * mapping, op_set * from, op_set * to, int dim, int * map, char const * name )
{
  initialise_map ( mapping, from, to, dim, map, name );
}

void op_decl_dat ( op_dat * data, op_set * set, int dim, int size, void *dat, char const * name )
{
  initialise_dat ( data, set, dim, size, dat, name );
}

