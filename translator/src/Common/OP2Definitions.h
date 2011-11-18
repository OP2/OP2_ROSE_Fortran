#pragma once
#ifndef OP2_DEFINITIONS_H
#define OP2_DEFINITIONS_H

#include <string>

class SgType;
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
  std::string const OP_DECL_SET = "op_decl_set";
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

    std::string const dataOnHost = "data";
    std::string const dataOnDevice = "data_d";
    std::string const dimension = "dim";
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
    std::string const warpSizeMacro = "OP_WARP_SIZE";
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
}

class OP2Definition
{
    /*
     * ======================================================
     * Base class to model an OP2 variable definition
     * ======================================================
     */

  protected:

    std::string variableName;

    OP2Definition ();

    virtual
    ~OP2Definition ();

  public:

    std::string const &
    getVariableName () const;
};

class OpDatDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_DAT variable definition
     * ======================================================
     */

  protected:

    std::string opSetName;

    unsigned int dimension;

    SgType * primitiveType;

  public:

    std::string const &
    getOpSetName () const;

    unsigned int
    getDimension () const;

    SgType *
    getPrimitiveType ();
};

class OpSetDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_SET variable definition
     * ======================================================
     */

  protected:

    std::string dimensionName;

  public:

    std::string const &
    getDimensionName () const;
};

class OpMapDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_MAP variable definition
     * ======================================================
     */

  protected:

    std::string sourceOpSetName;

    std::string destinationOpSetName;

    unsigned int dimension;

    std::string mappingName;

  public:

    std::string const &
    getSourceOpSetName () const;

    std::string const &
    getDestinationOpSetName () const;

    unsigned int
    getDimension () const;

    std::string const &
    getMappingName () const;
};

class OpGblDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_GBL variable definition
     * ======================================================
     */

  protected:

    unsigned int dimension;

    SgType * primitiveType;

  public:

    unsigned int
    getDimension () const;

    SgType *
    getPrimitiveType ();
};

class OpConstDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_CONST variable definition
     * ======================================================
     */

  protected:

    unsigned int dimension;

  public:

    unsigned int
    getDimension () const;
};

#endif
