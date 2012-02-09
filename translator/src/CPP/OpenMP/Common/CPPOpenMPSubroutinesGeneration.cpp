


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


#include "CPPOpenMPSubroutinesGeneration.h"
#include "CPPParallelLoop.h"
#include "CPPUserSubroutine.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPOpenMPHostSubroutineDirectLoop.h"
#include "CPPOpenMPKernelSubroutineDirectLoop.h"
#include "CPPOpenMPHostSubroutineIndirectLoop.h"
#include "CPPOpenMPKernelSubroutineIndirectLoop.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "OpenMP.h"
#include "OP2Definitions.h"

void
CPPOpenMPSubroutinesGeneration::addFreeVariableDeclarations ()
{
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Adding variables with constant access specifiers to module",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (map <string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    std::string const & variableName = it->first;

    OpConstDefinition * constDefinition = it->second;

    SgType * type = constDefinition->getType ();

    Debug::getInstance ()->debugMessage ("Analysing OP_DECL_CONST with name '"
        + variableName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    SgVariableDeclaration * variableDeclaration =
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, type, moduleScope);

    variableDeclaration->get_declarationModifier ().get_storageModifier ().setExtern ();
  }
}

void
CPPOpenMPSubroutinesGeneration::addHeaderIncludes ()
{
  using namespace SageInterface;

  addTextForUnparser (moduleScope, "#include \"" + OpenMP::CPP::libraryName
      + "\"\n", AstUnparseAttribute::e_before);

  addTextForUnparser (moduleScope, "#include \""
      + OpenMP::CPP::OP2RuntimeSupport + "\"\n", AstUnparseAttribute::e_before);
}

void
CPPOpenMPSubroutinesGeneration::createSubroutines ()
{
  using namespace SageInterface;
  using std::string;
  using std::map;

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    Debug::getInstance ()->debugMessage ("Analysing user subroutine '"
        + userSubroutineName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    CPPParallelLoop * parallelLoop =
        static_cast <CPPParallelLoop *> (it->second);

    CPPUserSubroutine * userSubroutine = new CPPUserSubroutine (moduleScope,
        parallelLoop, declarations);

    userSubroutines[userSubroutineName] = userSubroutine;

    CPPOpenMPKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      kernelSubroutine = new CPPOpenMPKernelSubroutineDirectLoop (moduleScope,
          userSubroutine, parallelLoop);

      hostSubroutines[userSubroutineName]
          = new CPPOpenMPHostSubroutineDirectLoop (moduleScope,
              kernelSubroutine, parallelLoop, moduleDeclarations);
    }
    else
    {
      kernelSubroutine = new CPPOpenMPKernelSubroutineIndirectLoop (
          moduleScope, userSubroutine, parallelLoop);

      hostSubroutines[userSubroutineName]
          = new CPPOpenMPHostSubroutineIndirectLoop (moduleScope,
              kernelSubroutine, parallelLoop, moduleDeclarations);
    }
  }
}

CPPOpenMPSubroutinesGeneration::CPPOpenMPSubroutinesGeneration (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPSubroutinesGeneration (project, declarations, "rose_openmp_code.cpp")
{
  generate ();
}
