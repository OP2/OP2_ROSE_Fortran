/*
 * Written by Adam Betts and Carlo Bertolli
 *  
 * Various public namespaces used in the compiler to declare and reference
 * variable names. Each namespace is generally split between Fortran-specific
 * and C++-specific names
 */

#ifndef COMMON_NAMESPACES_H
#define COMMON_NAMESPACES_H

#include <string>

namespace OP2
{
  /*
   * ======================================================
   * Declaration of names specific to OP2
   * ======================================================
   */

  std::string const OP_SET = "op_set";
  std::string const OP_DAT = "op_dat";
  std::string const OP_DAT_GBL = "op_dat_gbl";
  std::string const OP_MAP = "op_map";
  std::string const OP_GBL = "op_gbl";
  std::string const OP_ID = "op_id";
  std::string const OP_READ = "op_read";
  std::string const OP_WRITE = "op_write";
  std::string const OP_RW = "op_rw";
  std::string const OP_INC = "op_inc";
  std::string const OP_MAX = "op_max";
  std::string const OP_MIN = "op_min";
  std::string const OP_PAR_LOOP = "op_par_loop";
  std::string const OP_DECL_SET = "op_decl_set";
  std::string const OP_DECL_MAP = "op_decl_map";
  std::string const OP_DECL_DAT = "op_decl_dat";
  std::string const OP_DECL_GBL = "op_decl_gbl";
  std::string const OP_DECL_CONST = "op_decl_const";

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

  namespace CPP
  {
    std::string const OP_PTR = "op_ptr";
  }
}

namespace IndirectAndDirectLoop
{
  namespace Fortran
  {
    namespace ReductionSubroutine
    {
      namespace VariableNames
      {
        std::string const inputValue = "dat_l";
        std::string const iterationCounter = "i";
        std::string const reductionResultOnDevice = "dat_g";
        std::string const sharedMemoryStartOffset = "sharedMemoryStartOffset";
        std::string const threadID = "threadID";
        std::string const warpSize = "warpSize";
        std::string const maximumBytesInSharedMemory = "maxSharedMemoryBytes";
        std::string const maximumNumberOfThreadBlocks = "maxThreadBlocks";
        std::string const numberOfThreadItems = "threadItems";
        std::string const reductionArrayHost = "reductionArrayHost";
        std::string const reductionArrayDevice = "reductionArrayDevice";
      }

      namespace SubroutineNames
      {
        std::string const shift = "ishft";
      }
    }

    namespace HostSubroutine
    {
      std::string const subroutineName = "subroutineName";
      std::string const set = "set";
      std::string const size = "size";
      std::string const dim = "dim";
      std::string const dat = "dat";
      std::string const cudaThreadSynchronizeSubroutineName =
          "cudaThreadSynchronize";
      std::string const c2FortranPointerFunctionName = "c_f_pointer";
      std::string const c2FortranPointerVariablePrefix = "c2fPtrArg";
      std::string const threadSynchronizeSubroutineName =
          "cudaThreadSynchronize";
      std::string const threadSynchronizeReturnVariableName = "threadSynchRet";
      std::string const reductionIterationCounter1 = "i1Reduction";
      std::string const reductionIterationCounter2 = "i2Reduction";
    }

    namespace KernelSubroutine
    {
      std::string const offsetForReduction = "offsetReduction";
    }

    namespace VariableNames
    {
      std::string const argsSizes = "argsSizes";
      std::string const autoshared = "autoshared";
      std::string const opDatDimensions = "opDatDimensions";
    }

    namespace Libraries
    {
      std::string const ISO_C_BINDING = "ISO_C_BINDING";
      std::string const OP2_C = "OP2_C";
      std::string const CUDAFOR = "CUDAFOR";
      std::string const OMP_LIB = "OMP_LIB";
      std::string const cudaConfigurationParams = "cudaConfigurationParams";
    }
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
    namespace VariablePrefixes
    {
      std::string const numberOfBytes = "nBytes";
      std::string const pindMaps = "pindMaps";
      std::string const pMaps = "pMaps";
      std::string const roundUp = "roundUp";
    }

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

      std::string const constantsSeparator = "_";

      std::string const air_const = "air_const";
      std::string const alpha = "alpha";
      std::string const cfl = "cfl";
      std::string const eps = "eps";
      std::string const gam = "gam";
      std::string const gm1 = "gm1";
      std::string const mach = "mach";
      std::string const qinf = "qinf";
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
      namespace VariableNames
      {
        std::string const accesses = "accesses";
        std::string const actualPlan = "actualPlan";
        std::string const args = "args";
        std::string const argsNumber = "argsNumber";
        std::string const blockOffset = "blockOffset";
        std::string const col = "col";
        std::string const iterationCounter = "i";
        std::string const idxs = "idxs";
        std::string const inds = "inds";
        std::string const indsNumber = "indsNumber";
        std::string const maps = "maps";
        std::string const ncolblk = "ncolblk";
        std::string const planRet = "planRet";
      }

      namespace FieldNames
      {
        std::string const blkmap = "blkmap";
        std::string const ind_maps = "ind_maps";
        std::string const ind_offs = "ind_offs";
        std::string const ind_sizes = "ind_sizes";
        std::string const maps = "maps";
        std::string const nblocks = "nblocks";
        std::string const ncolblk = "ncolblk";
        std::string const ncolors = "ncolors";
        std::string const nelems = "nelems";
        std::string const nindirect = "nindirect";
        std::string const nshared = "nshared";
        std::string const nthrcol = "nthrcol";
        std::string const offset = "offset";
        std::string const size = "size";
        std::string const thrcol = "thrcol";
      }
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

  namespace CPP
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
      std::string const setSize = "setSize";
      std::string const warpScratchpadSize = "warpScratchpadSize";
      std::string const warpSize = "warpSize";
      std::string const setElementCounter = "n";
      std::string const dataPerElementCounter = "m";
      std::string const threadIDModulus = "tid";
      std::string const offsetInThreadBlock = "offset";
      std::string const remainingElements = "nelems";
      std::string const autosharedDisplacement = "autosharedDisplacement";
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
      std::string const blocksPerGrid = "nblocks";
      std::string const threadsPerBlock = "nthread";
      std::string const sharedMemorySize = "nshared";
      std::string const threadidx = "threadidx";
      std::string const blockidx = "blockidx";
      std::string const blockdim = "blockdim";
      std::string const griddim = "griddim";
    }

    namespace FieldNames
    {
      std::string const x = "x";
    }

    namespace SubroutineNames
    {
      std::string const syncthreads = "syncthreads";
    }

    namespace DirectLoop
    {
      /*
       * ======================================================
       * Following values are copied from Mike Giles'
       * implementation and may be subject to future changes
       * ======================================================
       */

      int const nblocks = 200;
      int const nthreads = 128;
    }
  }

  namespace CPP
  {
    namespace VariableNames
    {
    }

    namespace FieldNames
    {
    }
  }
}

namespace OpenMP
{
  std::string const numberOfThreads = "nThreads";
  std::string const threadIndex = "threadIndex";
  std::string const sliceStart = "sliceStart";
  std::string const sliceEnd = "sliceEnd";
  std::string const threadID = "threadID";
  std::string const sliceIterator = "sliceIterator";
  std::string const blockID = "blockID";
}

namespace TargetBackends
{
  /*
   * ======================================================
   * The different backends supported:
   * 1) CUDA
   * 2) OpenMP
   * 3) Unknown: primarily for debug reasons, to ensure the user
   *    is forced to select one of the above
   * ======================================================
   */

  std::string const CUDA = "CUDA";
  std::string const OpenMP = "OpenMP";
  std::string const Unknown = "Unknown";
}

namespace VariableNames
{
  /*
   * ======================================================
   * Returns the name of the formal parameter which models the
   * name of the user subroutine
   * ======================================================
   */
  std::string
  getUserSubroutineName ();

  /*
   * ======================================================
   * Returns the name of the formal parameter which models the
   * OP_SET
   * ======================================================
   */
  std::string
  getOpSetName ();

  /*
   * ======================================================
   * Returns the name of an OP_DAT variable in this OP_DAT
   * argument group
   * ======================================================
   */
  std::string
  getOpDatName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of a local OP_DAT variable in this
   * OP_DAT argument group
   * ======================================================
   */
  std::string
  getOpDatLocalName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of a global OP_DAT variable in this
   * OP_DAT argument group
   * ======================================================
   */
  std::string
  getOpDatGlobalName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the variable modelling the size of
   * an OP_DAT in this OP_DAT argument group
   * ======================================================
   */
  std::string
  getOpDatSizeName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the OP_DAT device variable
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string
  getOpDatDeviceName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the OP_DAT dimension variable
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string
  getOpDatDimensionName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the indirection variable in this
   * OP_DAT argument group
   * ======================================================
   */
  std::string
  getOpIndirectionName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the mapping variable in this OP_DAT
   * argument group
   * ======================================================
   */
  std::string
  getOpMapName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the access variable in this OP_DAT
   * argument group
   * ======================================================
   */
  std::string
  getOpAccessName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the C to Fortran variable
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string
  getCToFortranVariableName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Name of the field which represents the size of a
   * local to global renumbering (i.e. from local memory
   * into global device memory in the CUDA architecture)
   * argument used for an indirect OP_DAT.
   *
   * These are the 'ind_maps' variable sizes in the plan
   * function according to Mike Giles documentation
   * ======================================================
   */
  std::string
  getLocalToGlobalMappingName (unsigned int OP_DAT_ArgumentGroup);

  std::string
  getLocalToGlobalMappingSizeName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Name of the field which represents the size of a
   * global to local renumbering (i.e. from global device memory
   * to local memory in the CUDA architecture)
   * argument used for an indirect OP_DAT.
   *
   * These are the 'maps' variable sizes in the plan function
   * according to Mike Giles documentation
   * ======================================================
   */
  std::string
  getGlobalToLocalMappingName (unsigned int OP_DAT_ArgumentGroup);

  std::string
  getGlobalToLocalMappingSizeName (unsigned int OP_DAT_ArgumentGroup);

  std::string
  getNumberOfBytesVariableName (unsigned int OP_DAT_ArgumentGroup);

  std::string
  getRoundUpVariableName (unsigned int OP_DAT_ArgumentGroup);

  std::string
  getIncrementAccessMapName (unsigned int OP_DAT_ArgumentGroup);

  std::string
  getIncrementAccessMapName (unsigned int OP_DAT_ArgumentGroup);

  std::string
  getIndirectionArgumentSizeName (unsigned int OP_DAT_ArgumentGroup);
}

#endif
