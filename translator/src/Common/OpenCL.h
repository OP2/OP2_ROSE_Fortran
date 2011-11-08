#pragma once
#ifndef OPEN_CL_H
#define OPEN_CL_H

#include <string>

class SgFunctionCallExp;
class SgScopeStatement;

namespace OpenCL
{
  std::string const blocksPerGrid = "blocksPerGrid";
  std::string const threadsPerBlock = "threadsPerBlock";
  std::string const totalNumberOfThreads = "totalThreadNumber";
  std::string const sharedMemorySize = "dynamicSharedMemorySize";

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
}

#endif
