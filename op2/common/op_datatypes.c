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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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

static int check_set ( const op_set * set ) {
  return (set->index<0) || (set->index>=OP_set_index) ||
       strcmp(OP_set_list[set->index]->name,set->name);
}

void initialise_set ( op_set * set, int size, char const * name )
{
  assert(set);

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
  assert( mapping && from && to && map );

  if ( check_set( from ) ) {
    printf(" op_decl_map error -- invalid 'from' set for map %s\n",name);
    exit(-1);
  }

  if ( check_set( to ) ) {
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

  mapping->from = from;
  mapping->to   = to;
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

void initialise_dat ( op_dat * data,
                      int rank,
                      op_set * set0,
                      op_set * set1,
                      int data_rank,
                      int data_shape[4],
                      int size,
                      void *dat,
                      char const * name )
{
  assert( data && data_rank <= 4 );

  if ( rank < 0 || rank > 2 ) {
    printf("op_decl_dat error -- invalid rank for data: %s (0 <= rank <= 2)\n",name);
    exit(-1);
  }

  if ( rank > 0 && check_set(set0) || rank > 1 && check_set(set1) ) {
    printf("op_decl_dat error -- invalid set for data: %s\n",name);
    exit(-1);
  }

  int dim = 1;
  for (int i = 0; i < data_rank; ++i) {
    if (data_shape[i]<=0) {
      printf("op_decl_dat error -- negative/zero dimension for data: %s\n",name);
      exit(-1);
    }
    dim *= data_shape[i];
    data->data_shape[i] = data_shape[i];
  }

  data->rank = rank;
  data->set[0]= set0;
  data->set[1]= set1;
  data->dim   = dim;
  data->index = OP_dat_index;
  data->data_rank = data_rank;
  data->dat   = dat;
  data->dat_d = NULL;
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
  assert(map);

	map->from = NULL;
	map->to = NULL;
	map->dim = 0; //set to the proper value in Fortran
	map->index= -1; // no position in OP_map_list
	map->map = 0;
	map->name = "id";
}

void op_decl_gbl_map ( op_map * map )
{
  assert(map);

	map->from = NULL;
	map->to = NULL;
	map->dim = 0; //set to the proper value in Fortran
	map->index= -2; // no position in OP_map_list
	map->map = 0;
	map->name = "gbl";
}

op_arg op_construct_gbl_arg(op_dat * data, op_access acc)
{
  op_arg arg = (op_arg) {
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
    data,           /*arg.dat = data;*/
    { idx0, idx1 }, /*arg.idx = { idx0, idx1 };*/
    { map0, map1 }, /*arg.map = { map0, map1 };*/
    acc             /*arg.acc = acc;*/
  };

  return arg;
}

static void print_array_int( FILE* f, int const * v, size_t n ) {
  for (size_t i = 0; i < n; ++i)
    fprintf(f, "%d\n", v[i]);
  fprintf(f, "\n");
}

void dump_map ( op_map const * map, char const * filename ) {
  FILE *f = fopen(filename, "w");
  fprintf(f,"%s %d\n\n", map->name, map->dim);
  print_array_int(f, map->map, map->from->size * map->dim);
  fclose(f);
}

void dump_dat ( op_dat const * dat, char const * filename ) {
  FILE *f = fopen(filename, "w");
  fprintf(f,"%s %d %d %x %x\n", dat->name, dat->dim, dat->size, dat->dat, dat->dat_d);
  fclose(f);
}

void dump_sparsity ( op_sparsity const * sparsity, char const * filename ) {
  FILE *f = fopen(filename, "w");
  fprintf(f,"%zu %zu %zu\n\n", sparsity->nrows, sparsity->ncols, sparsity->max_nonzeros);
  print_array_int(f, sparsity->nnz, sparsity->nrows);
  print_array_int(f, sparsity->rowptr, sparsity->nrows + 1);
  print_array_int(f, sparsity->colidx, sparsity->rowptr[sparsity->nrows]);
  fclose(f);
}

