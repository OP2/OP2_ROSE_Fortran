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
  std::string const errorCode = "errorCode";
  std::string const event = "event";
  std::string const commandQueue = "commandQueue";
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
  getWorkGroupIDCallStatement (SgScopeStatement * scope,
      SgExpression * expression = NULL);

  namespace OP2RuntimeSupport
  {
    SgFunctionCallExp *
    getKernel (SgScopeStatement * scope, std::string const & kernelName);

    SgFunctionCallExp *
    getAssertMessage (SgScopeStatement * scope,
        SgExpression * assertExpression, SgStringVal * message);
  }
}

#endif
