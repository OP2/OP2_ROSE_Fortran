


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


#include <FortranCUDAModuleDeclarationsDirectLoop.h>
#include "FortranParallelLoop.h"
#include "FortranTypesBuilder.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranCUDAModuleDeclarations.h"
#include "ScopedVariableDeclarations.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include <rose.h>
#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp>


void
FortranCUDAModuleDeclarationsDirectLoop::createKernelArguments ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::map;
  using std::string;
  using std::vector;
  using boost::lexical_cast;
  
  /*
   * ======================================================
   * Create variables modelling the indirect mappings to
   * local indices in shared memory
   * Names are specialised with the user kernel name 
   * plus the check sum of op_dat arguments names
   * Redundant checks are for indirect loop (possibly to be
   * removed if this code remains here)
   * ======================================================
   */

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i) || parallelLoop->isIndirect (i))
      {

        string const & variableName = getOpDatDeviceName (i) +
          parallelLoop->getUserSubroutineName () + postfixName;

        Debug::getInstance ()->debugMessage (
          "Adding up: " + variableName,
          Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
          
        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getArray_RankOne (
                    parallelLoop->getOpDatBaseType (i)), moduleScope, 2,
                CUDA_DEVICE, ALLOCATABLE));
      }
    }
  }
}

FortranCUDAModuleDeclarationsDirectLoop::FortranCUDAModuleDeclarationsDirectLoop (
  FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
  FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
  FortranOpDatDimensionsDeclaration * dimensionsDeclaration) :
  FortranCUDAModuleDeclarations (parallelLoop, moduleScope, dataSizesDeclaration, dimensionsDeclaration)
{
  createKernelArguments();
}
