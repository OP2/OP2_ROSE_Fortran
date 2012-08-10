


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
    std::string const cPlanFunction = "FortranPlanCaller";
    std::string const fileName = "rose_openmp_code.F90";
  }

  std::string const sliceStart = "sliceStart";
  std::string const sliceEnd = "sliceEnd";
  std::string const numberOfThreads = "numberOfThreads";
  std::string const threadBlockID = "threadBlockID";
  std::string const threadBlockOffset = "threadBlockOffset";
  std::string const threadID = "threadID";
  
  
  
  std::string const
  getIfPartitionSizeDirectiveString ();  
  
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
  createGetThreadIDCallStatement (SgScopeStatement * scope);
  
  SgFunctionCallExp *
  createGetMaximumNumberOfThreadsCallStatement (SgScopeStatement * scope);

  std::string const
  getPrivateClause (std::vector <SgVarRefExp *> privateVariableReferences);
}

#endif
