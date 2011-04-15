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



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
