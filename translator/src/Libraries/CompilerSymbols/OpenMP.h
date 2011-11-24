#pragma once
#ifndef OPEN_MP_H
#define OPEN_MP_H

#include <string>
#include <vector>

class SgFunctionCallExp;
class SgScopeStatement;
class SgVarRefExp;

namespace OpenMP
{
  namespace CPP
  {
    std::string const libraryName = "omp.h";
    std::string const OP2RuntimeSupport = "op_openmp_rt_support.h";
  }

  namespace Fortran
  {
    std::string const libraryName = "OMP_LIB";
    std::string const cPlanFunction = "cplan_OpenMP";
  }

  std::string const sliceStart = "sliceStart";
  std::string const sliceEnd = "sliceEnd";
  std::string const numberOfThreads = "numberOfThreads";
  std::string const threadBlockID = "threadBlockID";
  std::string const threadBlockOffset = "threadBlockOffset";

  std::string const
  getIfDirectiveString ();

  std::string const
  getElseDirectiveString ();

  std::string const
  getEndIfDirectiveString ();

  std::string const
  getParallelLoopDirectiveString ();

  std::string const
  getEndParallelLoopDirectiveString ();

  SgFunctionCallExp *
  createGetMaximumNumberOfThreadsCallStatement (SgScopeStatement * scope);

  std::string const
  getPrivateClause (std::vector <SgVarRefExp *> privateVariableReferences);
}

#endif
