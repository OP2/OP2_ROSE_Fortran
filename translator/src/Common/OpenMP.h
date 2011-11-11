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
  std::string const threadIndex = "threadIndex";
  std::string const sliceStart = "sliceStart";
  std::string const sliceEnd = "sliceEnd";
  std::string const sliceIterator = "sliceIterator";
  std::string const numberOfThreads = "numberOfThreads";

  std::string const
  getIfDirectiveString ();

  std::string const
  getElseDirectiveString ();

  std::string const
  getEndIfDirectiveString ();

  std::string const
  getParallelForDirectiveString ();

  SgFunctionCallExp *
  createGetMaximumNumberOfThreadsCallStatement (SgScopeStatement * scope);

  std::string const
  getPrivateClause (std::vector <SgVarRefExp *> privateVariableReferences);
}

#endif
