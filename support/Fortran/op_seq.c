


//
// header files
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


#include <cuda.h>
#include <cuda_runtime_api.h>


#include "op_seq.h"
#include "op_support.h"

//
// global variables
//

int OP_set_index=0,
OP_map_index=0,
OP_dat_index=0,
OP_nplans   =0,
OP_diags    =0,
OP_part_size =0,
OP_block_size=512,
OP_cache_line_size=128;


op_set  * OP_set_list[MAX_SET_NUM];
op_map  * OP_map_list[MAX_MAP_NUM];
op_dat  * OP_dat_list[MAX_DAT_NUM];
op_plan   OP_plans[MAX_PLAN_NUM];
op_kernel OP_kernels[MAX_KERN_NUM];


/* The following null sets are used in the cplan and plan functions only (for now) */

// identity mapping
#define OP_ID  (op_map) {{0,0,"null"},{0,0,"null"},0,-1,NULL,"id"}

// global identifier
#define OP_GBL (op_map) {{0,0,"null"},{0,0,"null"},0,-2,NULL,"gbl"}





void op_decl_set(int size, op_set * set, char * name )
{
  set->size = size;
  set->name = name;
	
  set->index = OP_set_index;
  OP_set_list[OP_set_index++] = set;
}


void op_decl_map_f(op_set * from, op_set * to, int dim, int ** map, op_map * mapping, char * name )
{
  mapping->from = *from;
  mapping->to   = *to;
  mapping->dim  = dim;
  mapping->map  = *map;
	mapping->name = name;
	
  mapping->index = OP_map_index;
	
  OP_map_list[OP_map_index++] = mapping;
}


void op_decl_map(op_set from, op_set to, int dim, int * map, op_map * mapping, char * name )
{
  mapping->from = from;
  mapping->to   = to;
  mapping->dim  = dim;
  mapping->map  = map;
  mapping->name = name;
	
  mapping->index = OP_map_index;
	
  OP_map_list[OP_map_index++] = mapping;
}

void op_decl_null_map ( op_map * map )
{
	op_set nullSet = {0,0};
	
	map->from = nullSet;
	map->to = nullSet;	
	map->dim = 0; //set to the proper value in Fortran
	map->index= -1; // no position in OP_map_list
	map->map = NULL;
//	map->name = "null"; commented for now to avoid possible sigsegv
}



void op_decl_dat_f(op_set * set, int dim, char * type, int size, void **dat, op_dat *data, char * name)
{
  data->set   = *set;
  data->dim   = dim;
	data->type = type;
  data->dat   = (char *)*dat;
  data->size  = dim*size;
  data->index = OP_dat_index;
	data->name = name;
  OP_dat_list[OP_dat_index++] = data;	

  printf ( "Allocating device memory for data size %d and set size %d\n", data->size, set->size );

	cudaMalloc((void **)&(data->dat_d), data->size*set->size);
	cudaMemcpy((void *) data->dat_d, (void *) data->dat, data->size*set->size,
						 cudaMemcpyHostToDevice);
  cudaThreadSynchronize();
	
}

void op_decl_dat_scalar(op_set * set, int dim, char * type, int size, int dat, op_dat *data, char * name)
{
  data->set = NULL;
  data->dim = dim;
  data->type = type;

  data->dat = &dat;

  printf ( "Integer value is %d\n", (int) *(data->dat) );

  data->size  = dim*size;
  data->index = OP_dat_index;
	data->name = name;
  OP_dat_list[OP_dat_index++] = data;
	
}

void op_decl_dat(op_set set, int dim, char * type, int size, void *dat, op_dat *data, char * name)
{
  data->set   = set;
  data->dim   = dim;
	data->type = type;
  data->dat   = (char *) dat;
  data->size  = dim*size;
  data->index = OP_dat_index;
	data->name = name;
  OP_dat_list[OP_dat_index++] = data;
}

void op_decl_const_f ( int dim, void **dat, char const *name )
{
  if ( dim <= 0 )
	{
    printf ( "op_decl_const error -- negative/zero dimension for const: %s\n", name );
    exit ( -1 );
  }
}

void arg_set ( int displacement,
							 op_dat * arg,
							 int itemSel, // map field to be used
							 op_map * mapIn,
							 char ** p_arg)
{
  int n2;

  if ( mapIn->dim == -1 ) {					 // global variable, no mapping at all
		n2 = 0;
	}
	
  if ( mapIn->dim == 0 ) {					 // identity mapping
		n2 = displacement;
	}
	if ( mapIn->dim > 0 ) {             // standard pointers
    n2 = mapIn->map[itemSel + displacement * mapIn->dim];
	}
	
  *p_arg = (char *) (arg->dat + n2 * arg->size);                                                 
}                                                                                        






void op_par_loop_2 ( void (*subroutineName)(char *, char *), op_set * set,
										 op_dat * dat0, int itemSel0, op_map * map0, op_access access0,
										 op_dat * dat1, int itemSel1, op_map * map1, op_access access1
									 )
{
	int i;

	for ( i = 0; i < set->size; i++ ) {
		char * ptr0, * ptr1;
		
		arg_set ( i, dat0, itemSel0, map0, &ptr0 );
		arg_set ( i, dat1, itemSel1, map1, &ptr1 );
	
		(*subroutineName) ( ptr0, ptr1 );
	}
}

void op_par_loop_5 ( void (*subroutineName)(char *, char *, char *, char *, char *), op_set * set,
										op_dat * dat0, int itemSel0, op_map * map0, op_access access0,
										op_dat * dat1, int itemSel1, op_map * map1, op_access access1,
										op_dat * dat2, int itemSel2, op_map * map2, op_access access2,
										op_dat * dat3, int itemSel3, op_map * map3, op_access access3,
										op_dat * dat4, int itemSel4, op_map * map4, op_access access4
									 )
{
	int i;
	
	for ( i = 0; i < set->size; i++ ) {
		
		char * ptr0, * ptr1, * ptr2, * ptr3, * ptr4;
		
		arg_set ( i, dat0, itemSel0, map0, &ptr0 );
		arg_set ( i, dat1, itemSel1, map1, &ptr1 );
		arg_set ( i, dat2, itemSel2, map2, &ptr2 );
		arg_set ( i, dat3, itemSel3, map3, &ptr3 );
		arg_set ( i, dat4, itemSel4, map4, &ptr4 );
		
		
		(*subroutineName) ( ptr0, ptr1, ptr2, ptr3, ptr4 );
	}	
}

void op_par_loop_6 ( void (*subroutineName)(char *, char *, char *, char *, char *, char *), op_set * set,
										op_dat * dat0, int itemSel0, op_map * map0, op_access access0,
										op_dat * dat1, int itemSel1, op_map * map1, op_access access1,
										op_dat * dat2, int itemSel2, op_map * map2, op_access access2,
										op_dat * dat3, int itemSel3, op_map * map3, op_access access3,
										op_dat * dat4, int itemSel4, op_map * map4, op_access access4,
										op_dat * dat5, int itemSel5, op_map * map5, op_access access5
										)
{
	int i;
		
	for ( i = 0; i < set->size; i++ ) {

		char * ptr0, * ptr1, * ptr2, * ptr3, * ptr4, * ptr5;
		
		arg_set ( i, dat0, itemSel0, map0, &ptr0 );
		arg_set ( i, dat1, itemSel1, map1, &ptr1 );
		arg_set ( i, dat2, itemSel2, map2, &ptr2 );
		arg_set ( i, dat3, itemSel3, map3, &ptr3 );
		arg_set ( i, dat4, itemSel4, map4, &ptr4 );
		arg_set ( i, dat5, itemSel5, map5, &ptr5 );

		
		(*subroutineName) ( ptr0, ptr1, ptr2, ptr3, ptr4, ptr5 );

	}	
}

void op_par_loop_8 ( void (*subroutineName)(char *, char *, char *, char *, char *, char *, char *, char *), op_set * set,
										op_dat * dat0, int itemSel0, op_map * map0, op_access access0,
										op_dat * dat1, int itemSel1, op_map * map1, op_access access1,
										op_dat * dat2, int itemSel2, op_map * map2, op_access access2,
										op_dat * dat3, int itemSel3, op_map * map3, op_access access3,
										op_dat * dat4, int itemSel4, op_map * map4, op_access access4,
										op_dat * dat5, int itemSel5, op_map * map5, op_access access5,
										op_dat * dat6, int itemSel6, op_map * map6, op_access access6,
										op_dat * dat7, int itemSel7, op_map * map7, op_access access7
										)
{
	int i;
	
	for ( i = 0; i < set->size; i++ ) {
		
		char * ptr0, * ptr1, * ptr2, * ptr3, * ptr4, * ptr5, * ptr6, * ptr7;
		
		arg_set ( i, dat0, itemSel0, map0, &ptr0 );
		arg_set ( i, dat1, itemSel1, map1, &ptr1 );
		arg_set ( i, dat2, itemSel2, map2, &ptr2 );
		arg_set ( i, dat3, itemSel3, map3, &ptr3 );
		arg_set ( i, dat4, itemSel4, map4, &ptr4 );
		arg_set ( i, dat5, itemSel5, map5, &ptr5 );
		arg_set ( i, dat6, itemSel6, map6, &ptr6 );
		arg_set ( i, dat7, itemSel7, map7, &ptr7 );
		
		
		(*subroutineName) ( ptr0, ptr1, ptr2, ptr3, ptr4, ptr5, ptr6, ptr7 );
		
	}	
}


void op_par_loop_12 ( void (*subroutineName)(char *, char *, char *, char *, char *, char *, char *, char *, char *, char *, char *, char *),
		      op_set * set,
		      op_dat * dat0, int itemSel0, op_map * map0, op_access access0,
		      op_dat * dat1, int itemSel1, op_map * map1, op_access access1,
		      op_dat * dat2, int itemSel2, op_map * map2, op_access access2,
		      op_dat * dat3, int itemSel3, op_map * map3, op_access access3,
		      op_dat * dat4, int itemSel4, op_map * map4, op_access access4,
		      op_dat * dat5, int itemSel5, op_map * map5, op_access access5,
		      op_dat * dat6, int itemSel6, op_map * map6, op_access access6,
		      op_dat * dat7, int itemSel7, op_map * map7, op_access access7,
		      op_dat * dat8, int itemSel8, op_map * map8, op_access access8,
		      op_dat * dat9, int itemSel9, op_map * map9, op_access access9,
		      op_dat * dat10, int itemSel10, op_map * map10, op_access access10,
		      op_dat * dat11, int itemSel11, op_map * map11, op_access access11
		      )
{
	int i;
	
	for ( i = 0; i < set->size; i++ ) {
		
	  char * ptr0, * ptr1, * ptr2, * ptr3, * ptr4, * ptr5, * ptr6, * ptr7, * ptr8, * ptr9, * ptr10, * ptr11;
		
		arg_set ( i, dat0, itemSel0, map0, &ptr0 );
		arg_set ( i, dat1, itemSel1, map1, &ptr1 );
		arg_set ( i, dat2, itemSel2, map2, &ptr2 );
		arg_set ( i, dat3, itemSel3, map3, &ptr3 );
		arg_set ( i, dat4, itemSel4, map4, &ptr4 );
		arg_set ( i, dat5, itemSel5, map5, &ptr5 );
		arg_set ( i, dat6, itemSel6, map6, &ptr6 );
		arg_set ( i, dat7, itemSel7, map7, &ptr7 );
		arg_set ( i, dat8, itemSel8, map8, &ptr8 );
		arg_set ( i, dat9, itemSel9, map9, &ptr9 );
		arg_set ( i, dat10, itemSel10, map10, &ptr10 );
		arg_set ( i, dat11, itemSel11, map11, &ptr11 );
		
		
		(*subroutineName) ( ptr0, ptr1, ptr2, ptr3, ptr4, ptr5, ptr6, ptr7, ptr8, ptr9, ptr10, ptr11 );
		
	}	
}

// This function is used only by Fortran wrappers, then should be placed somewhere else
int getAccFromIntCode ( int accCode )
{
	switch ( accCode ) {
		case FOP_READ:
			return OP_READ;
		case FOP_WRITE:
			return OP_WRITE;
		case FOP_RW:
			return OP_RW;
		case FOP_INC:
			return OP_INC;
		default:
			return -1;
	}
}


//
//	wrapper from Fortran to plan function (see below)
//
op_plan * cplan ( char name[],
									int setId,
									int argsNumber,
									int args[],
									int idxs[],
									int maps[],
									int accs[],
									int indsNumber,
									int inds[]
								)
{
	
	int i, generatedPlanIndex = ERR;
	
	op_plan * generatedPlan = NULL;
	
	op_dat planArgs[argsNumber];
	op_map planMaps[argsNumber];
	int planDims[argsNumber];
	char * planTypes[argsNumber];
	op_access planAccs[argsNumber];
	op_set * iterationSet =  OP_set_list[setId];
	
	if ( iterationSet == NULL ) {
		printf ( "bad set index\n" );
		exit ( -1 );
	}
	
	// build planArgs variable by accessing OP_dat_list with indexes(=positions) in args
	for ( i = 0; i < argsNumber; i++ ) {
		op_dat * tmp = OP_dat_list[args[i]];
		planArgs[i] = *tmp;
	}

	// build planMaps variables by accessing OP_map_list with indexes(=positions) in args
	for ( i = 0; i < argsNumber; i++ ) {
		op_map * tmp;
		int j;
		
		if ( maps[i] != -1 ) { // another magic number !!!			
			int iter;
			tmp = OP_map_list[maps[i]];
			planMaps[i] = *tmp;			
		} else {
			// build false map with index = -1 ...
			planMaps[i] = OP_ID;
		}
	}
	
	// build dimensions of data using op_dat
	for ( i = 0; i < argsNumber; i++ ) {
		planDims[i] = planArgs[i].dim;
	}
	
	// build op_dat data type names (allocate precise space for name and copy it)
	for ( i = 0; i < argsNumber; i++ ) {
		// obtain reference to next op_dat
		op_dat * tmpDat = OP_dat_list[args[i]];
		
		// allocate space and copy strings 
		int typeNameLen =	strlen ( tmpDat->type );
		
		planTypes[i] = (char * ) calloc ( typeNameLen, sizeof ( char ) );
		strncpy ( planTypes[i], tmpDat->type, typeNameLen );
	}
	
	// build op_access array needed to build the plan
	for ( i = 0; i < argsNumber; i++ ){
		planAccs[i] = getAccFromIntCode ( accs[i] );
	}
	
	generatedPlan = plan ( name, *iterationSet, argsNumber, planArgs, idxs, planMaps, planDims, (const char **) planTypes, planAccs, indsNumber, inds);
	
//	printf ( "C Transfer = %f\n", generatedPlan->transfer );
	
	return generatedPlan;

}


void op_fetchdata ( op_dat * data ) {

  cudaMemcpy ( data->dat,
							 data->dat_d,
							 (data->size) * (data->set).size,
						   cudaMemcpyDeviceToHost
						 );
	
  cudaThreadSynchronize();
	
}



//int        *nthrcol;  // number of thread colors for each block
//int        *thrcol;   // thread colors
//int        *offset;   // offset for primary set
//int       **ind_maps; // pointers for indirect datasets
//int				 * nindirect; // size of each ind_maps position (for Fortran)
//int        *ind_offs; // offsets for indirect datasets
//int        *ind_sizes;// sizes for indirect datasets
//short     **maps;     // regular pointers, renumbered as needed
//int        *nelems;   // number of elements in each block
//int         ncolors;  // number of block colors
//int        *ncolblk;  // number of blocks for each color
//int					nblocks; // number of blocks (for Fortran)
//int        *blkmap;   // block mapping
//int         nshared;  // bytes of shared memory required
//float       transfer; // bytes of data transfer per kernel call
//float       transfer2;// bytes of cache line per kernel call



