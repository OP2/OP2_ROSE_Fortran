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
#include <rose.h>

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
}

namespace IndirectLoop
{
  namespace Fortran
  {
    namespace VariablePrefixes
    {
      std::string const numberOfBytes = "nBytes";
      std::string const pindMaps = "pindMaps";
      std::string const pMaps = "pMaps";
      std::string const roundUp = "roundUp";
    }

    namespace KernelSubroutine
    {
      namespace VariableNames
      {
        std::string const blockID = "blockID";
        std::string const col = "col";
        std::string const col2 = "col2";
        std::string const moduled = "moduled";
        std::string const moduloResult = "moduloResult";
        std::string const nbytes = "nbytes";
        std::string const ncolor = "ncolor";
        std::string const nelem = "nelem";
        std::string const nelems2 = "nelems2";
        std::string const offset_b = "offset_b";
      }
    }
  }
}

namespace DirectLoop
{
  namespace Fortran
  {
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
    std::string const blockidx = "blockidx";
    std::string const blockdim = "blockdim";
    std::string const cudaThreadSynchronize = "cudaThreadSynchronize";
    std::string const griddim = "griddim";
    std::string const blocksPerGrid = "nblocks";
    std::string const sharedMemorySize = "nshared";
    std::string const threadsPerBlock = "nthread";
    std::string const syncthreads = "syncthreads";
    std::string const threadidx = "threadidx";
    std::string const threadSynchRet = "threadSynchRet";
    std::string const x = "x";
  }
}

namespace OpenMP
{
  std::string const numberOfThreads = "nthreads";
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

namespace CommonVariableNames
{
  std::string const argsSizes = "argsSizes";
  std::string const autoshared = "autoshared";
  std::string const dat = "dat";
  std::string const dim = "dim";
  std::string const index = "index";
  std::string const iterationCounter1 = "i1";
  std::string const iterationCounter2 = "i2";
  std::string const opDatDimensions = "dimensions";
  std::string const set = "set";
  std::string const size = "size";
  std::string const upperBound = "n";
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

namespace SubroutineCalls
{

  /*
   * ======================================================
   * Returns a statement which represents a C-to-Fortran
   * pointer conversion
   * ======================================================
   */
  SgStatement *
  createCToFortranPointerCallStatement (SgScopeStatement * scope,
      SgExpression * parameter1, SgExpression * parameter2,
      SgExpression * parameter3 = NULL);
}

#endif
