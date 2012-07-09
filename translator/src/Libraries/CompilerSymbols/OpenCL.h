


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
#ifndef OPEN_CL_H
#define OPEN_CL_H

#include <string>

class SgType;
class SgFunctionCallExp;
class SgScopeStatement;
class SgIntVal;
class SgStringVal;
class SgVarRefExp;
class SgExpression;

namespace OpenCL
{
  std::string const blocksPerGrid = "blocksPerGrid";
  std::string const threadsPerBlock = "threadsPerBlock";
  std::string const totalThreadNumber = "totalThreadNumber";
  std::string const sharedMemorySize = "dynamicSharedMemorySize";
  std::string const CL_SUCCESS = "CL_SUCCESS";
  std::string const CLK_LOCAL_MEM_FENCE = "CLK_LOCAL_MEM_FENCE";
  std::string const errorCode = "errorCode";
  std::string const event = "event";
  std::string const commandQueue = "cqCommandQueue";
  std::string const kernelPointer = "kernelPointer";

  /*
   * ======================================================
   * The OpenCL type 'cl_kernel'
   * ======================================================
   */
  SgType *
  getKernelType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'cl_event'
   * ======================================================
   */
  SgType *
  getEventType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'cl_mem'
   * ======================================================
   */
  SgType *
  getMemoryType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'size_t'
   * ======================================================
   */
  SgType *
  getSizeType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'cl_short'
   * ======================================================
   */
  SgType *
  getSignedShortType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'cl_ushort'
   * ======================================================
   */
  SgType *
  getUnsignedShortType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'cl_int'
   * ======================================================
   */
  SgType *
  getSignedIntegerType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'cl_uint'
   * ======================================================
   */
  SgType *
  getUnsignedIntegerType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'cl_long'
   * ======================================================
   */
  SgType *
  getSignedLongType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'cl_ulong'
   * ======================================================
   */
  SgType *
  getUnsignedLongType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'cl_float'
   * ======================================================
   */
  SgType *
  getSinglePrecisionFloatType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'cl_double'
   * ======================================================
   */
  SgType *
  getDoublePrecisionFloatType (SgScopeStatement * scope);
  
  /*
   * ======================================================
   * Function call to set an OpenCL constant allocation
   * ======================================================
   */
  SgFunctionCallExp *
  getAllocateConstantExpression(SgScopeStatement * scope,
      std::string constantName, 
      SgVarRefExp * constant, SgType * constantType);

  /*
   * ======================================================
   * Function call to set an OpenCL kernel buffer 
   * argument
   * ======================================================
   */
  SgFunctionCallExp *
  getSetKernelArgumentCallBufferExpression (SgScopeStatement * scope,
      SgVarRefExp * openCLKernel, int argumentIndex, SgExpression * bufferRef);

  /*
   * ======================================================
   * Function call to set an OpenCL kernel actual
   * argument
   * ======================================================
   */
  SgFunctionCallExp *
  getSetKernelArgumentCallExpression (SgScopeStatement * scope,
      SgVarRefExp * openCLKernel, int argumentIndex, SgType * sizeOfArgument,
      SgExpression * argument = NULL);

  /*
   * ======================================================
   * Function call to enqueue an OpenCL kernel
   * ======================================================
   */
  SgFunctionCallExp *
  getEnqueueKernelCallExpression (SgScopeStatement * scope,
      SgVarRefExp * commandQueue, SgVarRefExp * openCLKernel,
      SgVarRefExp * globalWorkSize, SgVarRefExp * localWorkSize,
      SgVarRefExp * event);

  /*
   * ======================================================
   * Function call to finish OpenCL command queue
   * ======================================================
   */
  SgFunctionCallExp *
  getFinishCommandQueueCallExpression (SgScopeStatement * scope,
      SgVarRefExp * commandQueue);

  /*
   * ======================================================
   * Function call to OpenCL get_local_size.
   * When the given expression is NULL, this returns
   * get_loval_size(0) by default
   * ======================================================
   */
  SgFunctionCallExp *
  getLocalWorkGroupSizeCallStatement (SgScopeStatement * scope,
      SgExpression * expression = NULL);

  /*
   * ======================================================
   * Function call to OpenCL get_global_size.
   * When the given expression is NULL, this returns
   * get_global_size(0) by default
   * ======================================================
   */
  SgFunctionCallExp *
  getGlobalWorkGroupSizeCallStatement (SgScopeStatement * scope,
      SgExpression * expression = NULL);

  /*
   * ======================================================
   * Function call to OpenCL get_local_id.
   * When the given expression is NULL, this returns
   * get_local_id(0) by default
   * ======================================================
   */
  SgFunctionCallExp *
  getLocalWorkItemIDCallStatement (SgScopeStatement * scope,
      SgExpression * expression = NULL);

  /*
   * ======================================================
   * Function call to OpenCL get_global_id.
   * When the given expression is NULL, this returns
   * get_global_id(0) by default
   * ======================================================
   */
  SgFunctionCallExp *
  getGlobalWorkItemIDCallStatement (SgScopeStatement * scope, 
      SgExpression * expression = NULL);

  /*
   * ======================================================
   * Function call to OpenCL get_group_id.
   * ======================================================
   */
  SgFunctionCallExp *
  getWorkGroupIDCallStatement (SgScopeStatement * scope,
      SgExpression * expression = NULL);

  /*
   * ======================================================
   * Creates a barrier statement for all local work items
   * ======================================================
   */
  SgFunctionCallExp *
  createWorkItemsSynchronisationCallStatement (SgScopeStatement * scope);

/*  SgFunctionCallExp *
  getOpTimer (SgScopeStatement * scope, SgVarRefExp * cpuTime, SgVarRefExp * wallTime);
*/

//  SgFunctionCallExp *
//  getOpTimerCallStatement (SgScopeStatement * scope);

  namespace OP2RuntimeSupport
  {
    SgFunctionCallExp * 
    getOpTimerCallStatement (SgScopeStatement * scope,
    SgVarRefExp * cpuTime, SgVarRefExp * wallTime);

    SgFunctionCallExp *
    getOpTimingReallocCallStatement (SgScopeStatement * scope,
    SgExpression * index);

    SgFunctionCallExp *
    getKernel (SgScopeStatement * scope, std::string const & kernelName);

    SgFunctionCallExp *
    getAssertMessage (SgScopeStatement * scope,
        SgExpression * assertExpression, SgStringVal * message);

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
     * malloc (these are macros provided in the OP2 run-time
     * support)
     * ======================================================
     */
    SgVarRefExp *
    getPointerToMemoryAllocatedForDeviceReductionArray (
        SgScopeStatement * scope);
  }
}

#endif
