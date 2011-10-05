/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Helper class to get (opaque) variable references to variables
 * defined in the CUDA run-time library
 */

#pragma once
#ifndef CUDA_H
#define CUDA_H

#include <string>

class SgScopeStatement;
class SgDotExp;
class SgFunctionCallExp;

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
  std::string const blocksPerGrid = "nblocks";
  std::string const threadsPerBlock = "nthreads";
  std::string const sharedMemorySize = "nshared";
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
   * Returns a function call expression for thread
   * synchronisation to be called from a device subroutine
   * ======================================================
   */
  SgFunctionCallExp *
  createDeviceThreadSynchronisationCallStatement (SgScopeStatement * scope);

  /*
   * ======================================================
   * Returns a function call expression for thread
   * synchronisation to be called from a device subroutine
   * ======================================================
   */
  SgFunctionCallExp *
  createHostThreadSynchronisationCallStatement (SgScopeStatement * scope);
}

#endif
