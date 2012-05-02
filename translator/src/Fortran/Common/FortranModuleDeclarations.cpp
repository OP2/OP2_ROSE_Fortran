


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

#include <FortranParallelLoop.h>
#include <FortranModuleDeclarations.h>
#include <FortranTypesBuilder.h>
#include <ScopedVariableDeclarations.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include "CompilerGeneratedNames.h"
#include "OP2.h"
#include "Debug.h"
#include "Exceptions.h"
#include "rose.h"

void FortranModuleDeclarations::createPerformanceProfilingVariables ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
  
  string const & variableNameHost = loopTimeHost +
    parallelLoop->getUserSubroutineName () + postfixName;
  string const & variableNameKernel = loopTimeKernel +
    parallelLoop->getUserSubroutineName () + postfixName;
  string const & numberCalledVariableName = numberCalled +
    parallelLoop->getUserSubroutineName () + postfixName;


  Debug::getInstance ()->debugMessage ("Performance profiling, adding up: " + variableNameHost,
    Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  Debug::getInstance ()->debugMessage ("Performance profiling, adding up: " + variableNameKernel,
    Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  Debug::getInstance ()->debugMessage ("Performance profiling, adding up: " + numberCalledVariableName,
    Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (variableNameHost,
    FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
       variableNameHost, FortranTypesBuilder::getSinglePrecisionFloat (), moduleScope, 0));

  variableDeclarations->add (variableNameKernel,
    FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
       variableNameKernel, FortranTypesBuilder::getSinglePrecisionFloat (), moduleScope, 0));

  variableDeclarations->add (numberCalledVariableName,
    FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
       numberCalledVariableName, FortranTypesBuilder::getFourByteInteger (), moduleScope, 0));
}
			     

ScopedVariableDeclarations *
FortranModuleDeclarations::getDeclarations ()
{
  return variableDeclarations;
}

FortranModuleDeclarations::FortranModuleDeclarations (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  parallelLoop (parallelLoop), moduleScope (moduleScope)
{
  variableDeclarations = new ScopedVariableDeclarations ();

  /*
   * ======================================================
   * Eventually, put this into a command line option
   * ======================================================
   */
  createPerformanceProfilingVariables ();
}
