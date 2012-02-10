


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#pragma once
#ifndef OP2_H
#define OP2_H

#include <string>

class SgFunctionCallExp;
class SgExpression;
class SgScopeStatement;

namespace OP2
{
  /*
   * ======================================================
   * Declaration of names specific to OP2
   * ======================================================
   */

  std::string const OP_ACCESS = "op_access";
  std::string const OP_ARG = "op_arg";
  std::string const OP_ARG_DAT = "op_arg_dat";
  std::string const OP_ARG_GBL = "op_arg_gbl";
  std::string const OP_DAT = "op_dat";
  std::string const OP_DAT_CORE = "op_dat_core";
  std::string const OP_DAT_GBL = "op_dat_gbl";
  std::string const OP_DECL_CONST = "op_decl_const";
  std::string const OP_DECL_DAT = "op_decl_dat";
  std::string const OP_DECL_GBL = "op_decl_gbl";
  std::string const OP_DECL_MAP = "op_decl_map";
  std::string const OP_DECL_SET = "op_decl_set";\
  std::string const OP_DECL_SUBSET = "op_decl_subset";
  std::string const OP_GBL = "op_gbl";
  std::string const OP_ID = "op_id";
  std::string const OP_INC = "op_inc";
  std::string const OP_MAP = "op_map";
  std::string const OP_MAP_CORE = "op_map_core";
  std::string const OP_MAX = "op_max";
  std::string const OP_MIN = "op_min";
  std::string const OP_PAR_LOOP = "op_par_loop";
  std::string const OP_PLAN = "op_plan";
  std::string const OP_PLAN_GET = "op_plan_get";
  std::string const OP_READ = "op_read";
  std::string const OP_RW = "op_rw";
  std::string const OP_SET = "op_set";
  std::string const OP_SET_CORE = "op_set_core";
  std::string const OP_WRITE = "op_write";
  
  namespace RunTimeVariableNames
  {
    /*
     * ======================================================
     * Names hard-wired in the OP2 run-time support
     * ======================================================
     */

    std::string const dat = "dat";
    std::string const dat_d = "dat_d";
    std::string const data = "data";
    std::string const data_d = "data_d";
    std::string const dim = "dim";
    std::string const index = "index";
    std::string const set = "set";
    std::string const size = "size";

    namespace Fortran
    {
      std::string const dataPtr = "dataPtr";
      std::string const mapPtr = "mapPtr";
      std::string const setPtr = "setPtr";      
    }
  }

  namespace Macros
  {
    /*
     * ======================================================
     * Macros hard-wired in the OP2 run-time support
     * ======================================================
     */

    std::string const partitionSizeMacro = "OP_PARTITION_SIZE";
    std::string const threadBlockSizeMacro = "OP_BLOCK_SIZE";
    std::string const warpSizeMacro = "OP_WARPSIZE";
    std::string const fortranTypeOpGblMacro = "F_OP_ARG_GBL";
    std::string const fortranTypeOpDatMacro = "F_OP_ARG_DAT";

    /*
     * ======================================================
     * Returns a function call expression to the ROUND_UP
     * macro provided in the OP2 header files
     * ======================================================
     */

    SgFunctionCallExp *
    createRoundUpCallStatement (SgScopeStatement * scope,
        SgExpression * parameterExpression);

    /*
     * ======================================================
     * Returns a function call expression to the MAX
     * macro provided in the OP2 header files
     * ======================================================
     */

    SgFunctionCallExp *
    createMaxCallStatement (SgScopeStatement * scope,
        SgExpression * parameterExpression1,
        SgExpression * parameterExpression2);

    /*
     * ======================================================
     * Returns a function call expression to the MIN
     * macro provided in the OP2 header files
     * ======================================================
     */

    SgFunctionCallExp *
    createMinCallStatement (SgScopeStatement * scope,
        SgExpression * parameterExpression1,
        SgExpression * parameterExpression2);
  }

  namespace Libraries
  {
    namespace CPP
    {
      std::string const mainLibrary = "op_lib_cpp.h";
    }

    namespace Fortran
    {
      std::string const declarations = "OP2_FORTRAN_DECLARATIONS";
      std::string const runtimeSupport = "OP2_FORTRAN_RT_SUPPORT";
      std::string const CBindings = "ISO_C_BINDING";
    }
  }
  namespace FortranSpecific
  {
      namespace PrimitiveTypes
      {
        std::string const real8 = "real8";
        std::string const real4 = "real4";
        std::string const integer4 = "integer4";
      }
  }
}

#endif
