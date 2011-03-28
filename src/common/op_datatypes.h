
/*
 * OP datatypes
 */

#ifndef __OP_DATATYPES
#define __OP_DATATYPES

#ifdef __cplusplus
extern "C" {
#endif

/*
   0            none
   1 or above   error-checking
   2 or above   info on plan construction
   3 or above   report execution of parallel loops
   4 or above   report use of old plans
   7 or above   report positive checks in op_plan_check
*/

/*
 * enum list for op_par_loop
 */

enum op_access   { OP_READ, OP_WRITE, OP_RW, OP_INC, OP_MIN, OP_MAX };

/*
 * add in user's datatypes
 */

#ifdef OP_USER_DATATYPES
#include <OP_USER_DATATYPES>
#endif

/*
 * zero constants
 */

#define ZERO_double  0.0;
#define ZERO_float   0.0f;
#define ZERO_int     0;
#define ZERO_uint    0;
#define ZERO_ll      0;
#define ZERO_ull     0;
#define ZERO_bool    0;

/*
 * structures
 */

typedef struct {
  int         size,   /* number of elements in set */
              index;  /* index into list of sets  */
  char const *name;   /* name of set */
} op_set;

typedef struct {
  op_set      from,   /* set pointed from */
              to;     /* set pointed to */
  int         dim,    /* dimension of pointer */
              index,  /* index into list of pointers */
             *map;    /* array defining pointer */
  char const *name;   /* name of pointer */
} op_map;

typedef struct {
  op_set      set;    /* set on which data is defined */
  int         dim,    /* dimension of data */
              index,  /* index into list of datasets */
              size;   /* size of each element in dataset */
  void       *dat,    /* data on host */
             *dat_d;  /* data on device (GPU) */
  char const *name;   /* name of dataset */
} op_dat;

/* index for direct mapping */
#define OP_NONE -1
/* all indices selector */
#define OP_ALL -2

/* identity mapping */
#define OP_ID  (op_map) {{0,0,"null"},{0,0,"null"},0,0,NULL,"id"}

/* global identifier */
#define OP_GBL (op_map) {{0,0,"null"},{0,0,"null"},-1,0,NULL,"gbl"}

typedef struct {
  /* input arguments */
  char const  *name;
  int          set_index, nargs;
  int         *arg_idxs, *idxs, *map_idxs, *dims;
  char const **typs;
  enum op_access   *accs;

  /* execution plan */
  int        *nthrcol;  /* number of thread colors for each block */
  int        *thrcol;   /* thread colors */
  int        *offset;   /* offset for primary set */
  int       **ind_maps; /* pointers for indirect datasets */
  int        *ind_offs; /* offsets for indirect datasets */
  int        *ind_sizes;/* sizes for indirect datasets */
  short     **maps;     /* regular pointers, renumbered as needed */
  int        *nelems;   /* number of elements in each block */
  int         ncolors;  /* number of block colors */
  int        *ncolblk;  /* number of blocks for each color */
  int        *blkmap;   /* block mapping */
  int         nshared;  /* bytes of shared memory required */
  float       transfer; /* bytes of data transfer per kernel call */
  float       transfer2;/* bytes of cache line per kernel call */
} op_plan;

typedef struct {
  char const *name;     /* name of kernel function */
  int         count;    /* number of times called */
  float       time;     /* total execution time */
  float       transfer; /* bytes of data transfer (used) */
  float       transfer2;/* bytes of data transfer (total) */
} op_kernel;


/* 
 * Low-level initialisation functions
 */

void intialise_set ( op_set * set, int size, char const * name );

void intialise_map ( op_map * mapping, op_set * from, op_set * to, int dim, int * map, char const * name );

void intialise_dat ( op_dat * data, op_set * set, int dim, int type_size, void * dat, char const * name );

/*
 * Structure initialisation
 */

void op_decl_set ( op_set * set, int size, char const * name );

void op_decl_map ( op_map * mapping, op_set * from, op_set * to, int dim, int * map, char const * name );

void op_decl_dat ( op_dat * data, op_set * set, int dim, int type_size, void * dat, char const * name );

void op_decl_id_map ( op_map * map );

void op_decl_gbl_map ( op_map * map );

void op_decl_const ( void * dat, int dim, int type_size );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

