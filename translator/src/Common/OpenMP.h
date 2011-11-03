#pragma once
#ifndef OPEN_MP_H
#define OPEN_MP_H

#include <rose.h>

class SgFunctionCallExp;
class SgScopeStatement;

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

  SgFunctionCallExp *
  createGetMaximumNumberOfThreadsCallStatement (SgScopeStatement * scope);
}

#endif
