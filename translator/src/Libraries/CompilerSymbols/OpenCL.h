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
   * The OpenCL type 'cl_kernel'
   * ======================================================
   */
  SgType *
  getKernelType (SgScopeStatement * scope);

  /*
   * ======================================================
   * The OpenCL type 'size_t'
   * ======================================================
   */
  SgType *
  getSizeOfType (SgScopeStatement * scope);

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
      SgVarRefExp * openCLKernel, SgIntVal * argumentIndex,
      SgType * sizeOfArgument, SgExpression * argument = NULL);

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
    getAssertMessage (SgScopeStatement * scope, SgVarRefExp * successReference,
        SgStringVal * message);
  }
}

#endif
