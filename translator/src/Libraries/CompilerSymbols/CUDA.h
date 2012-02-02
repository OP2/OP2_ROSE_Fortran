


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
  namespace Libraries
  {
    namespace CPP
    {
      std::string const OP2RuntimeSupport = "op_cuda_rt_support.h";
    }

    namespace Fortran
    {
      std::string const CUDARuntimeSupport = "CUDAFOR";
      std::string const CUDALaunchParameters = "CUDACONFIGURATIONPARAMS";
    }
  }

  std::string const blocksPerGrid = "blocksPerGrid";
  std::string const threadsPerBlock = "threadsPerBlock";
  std::string const sharedMemorySize = "dynamicSharedMemorySize";
  std::string const threadSynchRet = "threadSynchRet";
  std::string const fortranCplanFunction = "cplan";

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

  namespace OP2RuntimeSupport
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
