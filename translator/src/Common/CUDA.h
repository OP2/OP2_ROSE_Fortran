#pragma once
#ifndef CUDA_H
#define CUDA_H

#include <string>

class SgScopeStatement;
class SgDotExp;
class SgFunctionCallExp;
class SgVarRefExp;

enum GRID_DIMENSION
{
  BLOCK_X, BLOCK_Y
};

enum THREAD_BLOCK_DIMENSION
{
  THREAD_X, THREAD_Y, THREAD_Z
};

namespace CUDA
{
  std::string const blocksPerGrid = "blocksPerGrid";
  std::string const threadsPerBlock = "threadsPerBlock";
  std::string const sharedMemorySize = "dynamicSharedMemorySize";
  std::string const threadSynchRet = "threadSynchRet";

  /*
   * ======================================================
   * Returns an opaque variable reference to either
   * threadIdx.x, threadIdx.y, or threadIdx.z
   * ======================================================
   */
  SgDotExp *
  getThreadId (THREAD_BLOCK_DIMENSION dimension, SgScopeStatement * scope);

  /*
   * ======================================================
   * Returns an opaque variable reference to either
   * blockIdx.x or blockIdx.y
   * ======================================================
   */
  SgDotExp *
  getBlockId (GRID_DIMENSION dimension, SgScopeStatement * scope);

  /*
   * ======================================================
   * Returns an opaque variable reference to either
   * blockDim.x, blockDim.y, or blockDim.z
   * ======================================================
   */
  SgDotExp *
  getThreadBlockDimension (THREAD_BLOCK_DIMENSION dimension,
      SgScopeStatement * scope);

  /*
   * ======================================================
   * Returns an opaque variable reference to either
   * gridDim.x or gridDim.y
   * ======================================================
   */
  SgDotExp *
  getGridDimension (GRID_DIMENSION dimension, SgScopeStatement * scope);

  /*
   * ======================================================
   * Returns an opaque variable reference to the warp size
   * variable
   * ======================================================
   */
  SgVarRefExp *
  getWarpSizeReference (SgScopeStatement * scope);

  /*
   * ======================================================
   * Returns a function call expression for thread
   * synchronisation to be called from a device subroutine
   * ======================================================
   */
  SgFunctionCallExp *
  createDeviceThreadSynchronisationCallStatement (SgScopeStatement * scope);

  /*
   * ======================================================
   * Returns a function call expression for thread
   * synchronisation to be called from a host subroutine
   * ======================================================
   */
  SgFunctionCallExp *
  createHostThreadSynchronisationCallStatement (SgScopeStatement * scope);

  namespace CPPRuntimeSupport
  {
    /*
     * ======================================================
     * Returns a function call expression to the run-time
     * support function which allocates reduction arrays
     * ======================================================
     */
    SgFunctionCallExp *
    getReallocateReductionArraysCallStatement (SgScopeStatement * scope,
        SgVarRefExp * reductionBytesReference);

    /*
     * ======================================================
     * Returns a function call expression to the run-time
     * support function which moves reduction arrays from host
     * to device
     * ======================================================
     */
    SgFunctionCallExp *
    getMoveReductionArraysFromHostToDeviceCallStatement (
        SgScopeStatement * scope, SgVarRefExp * reductionBytesReference);

    /*
     * ======================================================
     * Returns a function call expression to the run-time
     * support function which moves reduction arrays from host
     * to device
     * ======================================================
     */
    SgFunctionCallExp *
    getMoveReductionArraysFromDeviceToHostCallStatement (
        SgScopeStatement * scope, SgVarRefExp * reductionBytesReference);

    /*
     * ======================================================
     * Returns a function call expression for thread
     * synchronisation to be called from a C++ host subroutine,
     * which itself is wrapped in CUDA safe call function call
     * ======================================================
     */
    SgFunctionCallExp
        *
        getCUDASafeHostThreadSynchronisationCallStatement (
            SgScopeStatement * scope);

    /*
     * ======================================================
     * Returns a reference to the pointer which is returned
     * after allocating a reduction array on the host through
     * malloc (these are macros provided in the OP2 run-time
     * support)
     * ======================================================
     */
    SgVarRefExp *
    getPointerToMemoryAllocatedForHostReductionArray (SgScopeStatement * scope);

    /*
     * ======================================================
     * Returns a reference to the pointer which is returned
     * after allocating a reduction array on the device through
     * CUDA malloc (these are macros provided in the OP2 run-time
     * support)
     * ======================================================
     */
    SgVarRefExp *
    getPointerToMemoryAllocatedForDeviceReductionArray (
        SgScopeStatement * scope);
  }
}

#endif
