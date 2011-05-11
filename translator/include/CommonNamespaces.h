/*
 * Written by Adam Betts and Carlo Bertolli
 *  
 * Various public namespaces used in the compiler
 */

#ifndef COMMON_NAMESPACES_H
#define COMMON_NAMESPACES_H

namespace OP2
{
  /*
   * ======================================================
   * Declaration of names specific to OP2
   * ======================================================
   */

  std::string const OP_SET_NAME = "op_set";
  std::string const OP_DAT_NAME = "op_dat";
  std::string const OP_MAP_NAME = "op_map";
  std::string const OP_ID_NAME = "op_id";
  std::string const OP_GBL_NAME = "op_gbl";
  std::string const OP_READ_NAME = "op_read";
  std::string const OP_WRITE_NAME = "op_write";
  std::string const OP_RW_NAME = "op_rw";
  std::string const OP_INC_NAME = "op_inc";
  std::string const OP_MAX_NAME = "op_max";
  std::string const OP_MIN_NAME = "op_min";
  std::string const OP_PAR_LOOP_PREFIX = "op_par_loop";
  std::string const OP_DECL_SET_PREFIX = "op_decl_set";
  std::string const OP_DECL_MAP_PREFIX = "op_decl_map";
  std::string const OP_DECL_DAT_PREFIX = "op_decl_dat";
  std::string const OP_DECL_GBL_PREFIX = "op_decl_gbl";

  namespace Fortran
  {
    /*
     * ======================================================
     * The following constants assume that each call to an
     * OP_PAR_LOOP in Fortran is constructed as follows:
     *
     * OP_PAR_LOOP_<N>
     *   (nameOfUserSubroutine,
     *    OP_SET,
     *    OP_DAT,   int,   OP_MAP,   OP_ACCESS,
     *    ...
     *    OP_DAT_N, int_N, OP_MAP_N, OP_ACCESS_N)
     * ======================================================
     */

    /*
     * ======================================================
     * Currently, for every OP_DAT, there is an int, an OP_MAP,
     * and an OP_ACCESS
     * ======================================================
     */
    unsigned int const NUMBER_OF_ARGUMENTS_PER_OP_DAT = 4;

    /*
     * ======================================================
     * There are 2 non-OP_DAT arguments, namely
     * 'nameOfUserSubroutine' and 'OP_SET'
     * ======================================================
     */
    unsigned int const NUMBER_OF_NON_OP_DAT_ARGUMENTS = 2;
  }

  namespace CPlusPlus
  {

  }
}

namespace IndirectAndDirectLoop
{
  namespace HostSubroutine
  {
    std::string const threadSynchRet = "threadSynchRet";
  }
}

namespace IndirectLoop
{
  /*
   * ======================================================
   * The names used in the indirect loop implementation
   * ======================================================
   */

  namespace Fortran
  {
    namespace ConstantSuffixes
    {
      /*
       * ======================================================
       * Following are suffixes of symbolic constants needed
       * in each indirect loop. They are suffixes because
       * Fortran does not have a namespace concept, so we
       * prepend the name of the user device subroutine to make
       * them unique.
       *
       * Although these constants are used across all modules
       * implementing an OP_PAR_LOOP (for each indirect loop),
       * we cannot have a single module in which they are
       * included and initialised. This is because currently
       * the compilers generating the final binaries (e.g.
       * PGI) do not support calling functions/subroutines
       * outside of a single Fortran module.
       * ======================================================
       */

      std::string const air_const = "_air_const";
      std::string const alpha = "_alpha";
      std::string const cfl = "_cfl";
      std::string const eps = "_eps";
      std::string const gam = "_gam";
      std::string const gm1 = "_gm1";
      std::string const mach = "_mach";
      std::string const qinf = "_qinf";
    }

    namespace PlanFunction
    {
      /*
       * ======================================================
       * Following are variable names used in the plan function
       * returned for an indirect loop.
       * ======================================================
       */

      namespace VariableNames
      {
        std::string const blockOffset = "blockOffset";
        std::string const pblkMap = "pblkMap";
        std::string const pindMaps = "pindMaps";
        std::string const pindOffs = "pindOffs";
        std::string const pindSizes = "pindSizes";
        std::string const poffset = "poffset";
        std::string const pmaps = "pmaps";
        std::string const pnelems = "pnelems";
        std::string const pnindirect = "pnindirect";
        std::string const pnthrcol = "pnthrcol";
        std::string const pthrcol = "pthrcol";
        std::string const pblkMapSize = "pblkMapSize";
        std::string const pindMapsSize = "pindMapsSize";
        std::string const pindOffsSize = "pindOffsSize";
        std::string const pindSizesSize = "pindSizesSize";
        std::string const poffsetSize = "poffsetSize";
        std::string const pnelemsSize = "pnelemsSize";
        std::string const pnthrcolSize = "pnthrcolSize";
        std::string const pthrcolSize = "pthrcolSize";
      }
    }

    namespace HostSubroutine
    {
    }

    namespace KernelSubroutine
    {
      namespace VariableNames
      {
        std::string const blockID = "blockID";
        std::string const col = "col";
        std::string const col2 = "col2";
        std::string const iterationCounter = "i";
        std::string const iterationCounter2 = "i2";
        std::string const moduled = "moduled";
        std::string const moduloResult = "moduloResult";
        std::string const nbytes = "nbytes";
        std::string const ncolor = "ncolor";
        std::string const nelem = "nelem";
        std::string const nelems2 = "nelems2";
        std::string const offset_b = "offset_b";
        std::string const whileLoopBound = "whileLoopBound";
      }
    }
  }

  namespace CPlusPlus
  {
    namespace PlanFunction
    {
    }

    namespace HostSubroutine
    {

    }

    namespace KernelSubroutine
    {

    }
  }
}

namespace DirectLoop
{
  /*
   * ======================================================
   * The names used in the direct loop implementation
   * ======================================================
   */

  namespace Fortran
  {
    namespace PlanFunction
    {
    }

    namespace HostSubroutine
    {
      std::string const threadSynchRet = "threadSynchRet";
    }

    namespace KernelSubroutine
    {

    }
  }

  namespace CPlusPlus
  {
    namespace PlanFunction
    {
    }

    namespace HostSubroutine
    {

    }

    namespace KernelSubroutine
    {

    }
  }
}

namespace CUDA
{
  /*
   * ======================================================
   * The names used in the CUDA libraries
   * ======================================================
   */

  namespace Fortran
  {
    namespace VariableNames
    {
      std::string const threadidx = "threadidx";
      std::string const blockidx = "blockidx";
      std::string const blockdim = "blockdim";
      std::string const griddim = "griddim";
    }

    namespace FieldNames
    {
      std::string const x = "x";
    }
  }

  namespace CPlusPlus
  {
    namespace VariableNames
    {
    }

    namespace FieldNames
    {
    }
  }
}

namespace OtherVariableNames
{
  std::string const argsSizes = "argsSizes";
  std::string const autoshared = "autoshared";
}

namespace VariablePrefixes
{
  std::string const OP_DAT_Name = "argument";
  std::string const localVarNamePrefix = "arg";
}

namespace VariableSuffixes
{
  std::string const Size = "Size";
  std::string const localVarName = "_l";
}

namespace ReductionSubroutineNames
{

  std::string const iterationVarForReductionName1 = "iterationRed1";
  std::string const iterationVarForReductionName2 = "iterationRed2";
}

namespace FortranVariableDeafultKinds
{
  int const DEFAULT_KIND_INT = 4;
  int const DEFAULT_KIND_REAL = 4;
}

#endif
