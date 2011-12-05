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
   * The OpenCL type 'cl_double'
   * ======================================================
   */
  SgFunctionCallExp *
  getSetKernelArgumentCallExpression (SgScopeStatement * scope,
      SgVarRefExp * openCLKernel, int argumentIndex, SgType * sizeOfArgument,
      SgExpression * argument = NULL);

  SgFunctionCallExp *
  getWorkGroupDimensionsCallStatement (SgScopeStatement * scope);

  SgFunctionCallExp *
  getLocalWorkGroupSizeCallStatement (SgScopeStatement * scope);

  SgFunctionCallExp *
  getGlobalWorkGroupSizeCallStatement (SgScopeStatement * scope);

  SgFunctionCallExp *
  getLocalWorkItemIDCallStatement (SgScopeStatement * scope);

  SgFunctionCallExp *
  getWorkGroupIDCallStatement (SgScopeStatement * scope);

  SgFunctionCallExp *
  getNumberOfWorkGroupsCallStatement (SgScopeStatement * scope);

  namespace OP2RuntimeSupport
  {
    SgFunctionCallExp *
    getKernel (SgScopeStatement * scope, std::string const & kernelName);

    SgFunctionCallExp *
    getAssertMessage (SgScopeStatement * scope, SgVarRefExp * successReference,
        SgStringVal * message);
  }
}

#endif
