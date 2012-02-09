


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


#include "CPPOpenCLSubroutinesGeneration.h"
#include "CPPParallelLoop.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPOpenCLKernelSubroutineDirectLoop.h"
#include "CPPOpenCLKernelSubroutineIndirectLoop.h"
#include "CPPOpenCLHostSubroutineDirectLoop.h"
#include "CPPOpenCLHostSubroutineIndirectLoop.h"
#include "CPPOpenCLUserSubroutine.h"
#include "CPPOpenCLReductionSubroutine.h"
#include "CPPReductionSubroutines.h"
#include "CPPOpenCLConstantDeclarations.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "OP2Definitions.h"
#include "OP2.h"

void
CPPOpenCLSubroutinesGeneration::addFreeVariableDeclarations ()
{
  constantDeclarations = new CPPOpenCLConstantDeclarations (declarations,
      moduleScope);
}

void
CPPOpenCLSubroutinesGeneration::createReductionSubroutines ()
{
  using boost::lexical_cast;
  using std::string;
  using std::map;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Creating reduction subroutines",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <Reduction *> reductionsNeeded;

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    ParallelLoop * parallelLoop = it->second;

    parallelLoop->getReductionsNeeded (reductionsNeeded);
  }

  for (vector <Reduction *>::const_iterator it = reductionsNeeded.begin (); it
      != reductionsNeeded.end (); ++it)
  {
    CPPOpenCLReductionSubroutine * subroutine =
        new CPPOpenCLReductionSubroutine (moduleScope, *it);

    reductionSubroutines->addSubroutine (*it,
        subroutine->getSubroutineHeaderStatement ());
  }
}

void
CPPOpenCLSubroutinesGeneration::createSubroutines ()
{
  using std::string;
  using std::map;

  createReductionSubroutines ();

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    Debug::getInstance ()->debugMessage ("Analysing user subroutine '"
        + userSubroutineName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    CPPParallelLoop * parallelLoop =
        static_cast <CPPParallelLoop *> (it->second);

    CPPOpenCLUserSubroutine * userDeviceSubroutine =
        new CPPOpenCLUserSubroutine (moduleScope, parallelLoop, declarations);

    userSubroutines[userSubroutineName] = userDeviceSubroutine;

    CPPOpenCLKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      kernelSubroutine = new CPPOpenCLKernelSubroutineDirectLoop (moduleScope,
          userDeviceSubroutine, parallelLoop, reductionSubroutines,
          declarations);

      hostSubroutines[userSubroutineName]
          = new CPPOpenCLHostSubroutineDirectLoop (moduleScope,
              kernelSubroutine, parallelLoop, moduleDeclarations,
              userDeviceSubroutine, constantDeclarations);
    }
    else
    {
      kernelSubroutine = new CPPOpenCLKernelSubroutineIndirectLoop (
          moduleScope, userDeviceSubroutine, parallelLoop,
          reductionSubroutines, declarations);

      hostSubroutines[userSubroutineName]
          = new CPPOpenCLHostSubroutineIndirectLoop (moduleScope,
              kernelSubroutine, parallelLoop, moduleDeclarations,
              userDeviceSubroutine, constantDeclarations);
    }
  }
}

void
CPPOpenCLSubroutinesGeneration::addHeaderIncludes ()
{
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Adding '#include' statements to main file", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  addTextForUnparser (moduleScope, "\n#include <CL/cl.h>\n",
      AstUnparseAttribute::e_after);

  addTextForUnparser (moduleScope, "#include \""
      + OP2::Libraries::CPP::mainLibrary + "\"\n",
      AstUnparseAttribute::e_before);
}

CPPOpenCLSubroutinesGeneration::CPPOpenCLSubroutinesGeneration (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPSubroutinesGeneration (project, declarations, "rose_opencl_code.cpp")
{
  generate ();
}
