// This is the header file needed by our source-to-source translator to 
// correctly parse C++ programs with OP2 calls. The OP2 syntax is the one
// suggested by Imperial College to remove some of the superfluous arguments  

#ifndef __OP2_IMPERIAL_H
#define __OP2_IMPERIAL_H
typedef struct
{
} op_set;

typedef struct
{
} op_map;

typedef struct 
{
} op_dat;

typedef struct 
{
} op_arg;

typedef struct
{
} op_sparsity;

typedef struct
{
} op_mat;

typedef struct
{
} op_itspace;

typedef enum {OP_READ, OP_WRITE, OP_INC, OP_RW, OP_MIN, OP_MAX} op_access;

op_map const OP_ID;
op_map const OP_GBL;

#define OP_ALL -2

void 
op_init(int, char **, int);

void
op_exit ();

op_set 
op_decl_set (int);

op_map 
op_decl_map (op_set, op_set, int, int *);

template <typename T>
op_dat 
op_decl_dat (op_set, int, T *);

template <typename T>
void
op_decl_const (int, T *);

op_sparsity
op_decl_sparsity(op_map, op_map);

op_mat
op_decl_mat(op_sparsity, int, char *);

op_arg
op_arg_dat(op_dat, int, op_map, op_access);

op_arg
op_arg_mat(op_mat, int, op_map, int, op_map, op_access);

template <typename T>
op_arg
op_arg_gbl(T *, op_access);

void
op_diagnostic_output();

void
op_timing_output();

op_itspace
op_iteration_space (op_set, int, int);

int op_i(int);

// 1 OP_ARG
template <typename T1>
void
op_par_loop (void (*kernel) (T1 *), 
             op_set, 
             op_arg);

// 2 OP_ARG
template <typename T1, typename T2>
void
op_par_loop (void (*kernel) (T1 *, T2 *), 
             op_set, 
             op_arg, 
             op_arg);

// 3 OP_ARG
template <typename T1, typename T2, typename T3>
void
op_par_loop (void (*kernel) (T1 *, T2 *, T3 *), 
             op_set, 
             op_arg, 
             op_arg,
             op_arg);

// 4 OP_ARG
template <typename T1, typename T2, typename T3, typename T4>
void
op_par_loop (void (*kernel) (T1 *, T2 *, T3 *, T4 *), 
             op_set, 
             op_arg, 
             op_arg,
             op_arg,
             op_arg);

// 5 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void
op_par_loop (void (*kernel) (T1 *, T2 *, T3 *, T4 *, T5 *), 
             op_set, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg);

// 6 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void
op_par_loop (void (*kernel) (T1 *, T2 *, T3 *, T4 *, T5 *, T6 *), 
             op_set, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg);

// 7 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void
op_par_loop (void (*kernel) (T1 *, T2 *, T3 *, T4 *, T5 *, T6 *, T7 *), 
             op_set, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg,
             op_arg);

// 8 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void
op_par_loop (void (*kernel) (T1 *, T2 *, T3 *, T4 *, T5 *, T6 *, T7 *, T8 *), 
             op_set, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg);

// 9 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
void
op_par_loop (void (*kernel) (T1 *, T2 *, T3 *, T4 *, T5 *, T6 *, T7 *, T8 *, T9 *), 
             op_set, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg);

// 10 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
void
op_par_loop (void (*kernel) (T1 *, T2 *, T3 *, T4 *, T5 *, T6 *, T7 *, T8 *, T9 *, T10 *), 
             op_set, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg,
             op_arg);

// 1 OP_ARG
template <typename T1>
void
op_par_loop (void (*kernel) (T1, int, int), 
             char *,
             op_itspace, 
             op_arg);

// 2 OP_ARG
template <typename T1, typename T2>
void
op_par_loop (void (*kernel) (T1, T2, int, int), 
             char *,
             op_itspace, 
             op_arg, 
             op_arg);

// 3 OP_ARG
template <typename T1, typename T2, typename T3>
void
op_par_loop (void (*kernel) (T1, T2, T3, int, int), 
             char *,
             op_itspace, 
             op_arg, 
             op_arg,
             op_arg);

// 4 OP_ARG
template <typename T1, typename T2, typename T3, typename T4>
void
op_par_loop (void (*kernel) (T1, T2, T3, T4, int, int), 
             char *,
             op_itspace, 
             op_arg, 
             op_arg,
             op_arg,
             op_arg);

// 5 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5>
void
op_par_loop (void (*kernel) (T1, T2, T3, T4, T5, int, int), 
             char *,
             op_itspace, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg);

// 6 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void
op_par_loop (void (*kernel) (T1, T2, T3, T4, T5, T6, int, int), 
             char *,
             op_itspace, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg);

// 7 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void
op_par_loop (void (*kernel) (T1, T2, T3, T4, T5, T6, T7, int, int), 
             char *,
             op_itspace, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg,
             op_arg);

// 8 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
void
op_par_loop (void (*kernel) (T1, T2, T3, T4, T5, T6, T7, T8, int, int), 
             char *,
             op_itspace, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg);

// 9 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
void
op_par_loop (void (*kernel) (T1, T2, T3, T4, T5, T6, T7, T8, T9, int, int), 
             char *,
             op_itspace, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg);

// 10 OP_ARG
template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
void
op_par_loop (void (*kernel) (T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, int, int), 
             char *,
             op_itspace, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg, 
             op_arg,
             op_arg);

#endif  /* __OP2_IMPERIAL_H */
