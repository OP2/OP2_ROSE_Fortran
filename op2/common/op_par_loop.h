
#ifndef __OP_PAR_LOOP_H
#define __OP_PAR_LOOP_H

#include "op_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void op_par_loop_1 ( void (*kernel)(void *), char const * name, op_set * set,
                     op_arg arg0
                   );

void op_par_loop_2 ( void (*kernel)(void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1
                   );

void op_par_loop_3 ( void (*kernel)(void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2
                   );

void op_par_loop_4 ( void (*kernel)(void *, void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2,
                     op_arg arg3
                   );

void op_par_loop_5 ( void (*kernel)(void *, void *, void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2,
                     op_arg arg3,
                     op_arg arg4
                   );

void op_par_loop_6 ( void (*kernel)(void *, void *, void *, void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2,
                     op_arg arg3,
                     op_arg arg4,
                     op_arg arg5
                   );

void op_par_loop_7 ( void (*kernel)(void *, void *, void *, void *, void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2,
                     op_arg arg3,
                     op_arg arg4,
                     op_arg arg5,
                     op_arg arg6
                   );

void op_par_loop_8 ( void (*kernel)(void *, void *, void *, void *, void *, void *, void *, void *), char const * name, op_set * set,
                     op_arg arg0,
                     op_arg arg1,
                     op_arg arg2,
                     op_arg arg3,
                     op_arg arg4,
                     op_arg arg5,
                     op_arg arg6,
                     op_arg arg7
                   );



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
