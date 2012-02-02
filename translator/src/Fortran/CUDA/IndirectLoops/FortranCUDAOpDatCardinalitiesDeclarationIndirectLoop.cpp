


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


#include "FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop.h"
#include "FortranParallelLoop.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "ScopedVariableDeclarations.h"
#include "PlanFunctionNames.h"
#include <rose.h>

void
FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop::addIndirectLoopFields ()
{
  using namespace SageBuilder;
  using namespace PlanFunctionVariableNames;
  using std::string;
  using std::vector;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isDuplicateOpDat (i) == false)
      {

        string const variableName1 = getLocalToGlobalMappingSizeName (i);

        SgVariableDeclaration * fieldDeclaration1 = buildVariableDeclaration (
            variableName1, FortranTypesBuilder::getFourByteInteger (), NULL,
            moduleScope);

        fieldDeclaration1->get_declarationModifier ().get_accessModifier ().setUndefined ();

        deviceDatatypeStatement->get_definition ()->append_member (
            fieldDeclaration1);

        fieldDeclarations->add (variableName1, fieldDeclaration1);
      }

      string const variableName2 = getGlobalToLocalMappingSizeName (i);

      SgVariableDeclaration * fieldDeclaration2 = buildVariableDeclaration (
          variableName2, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration2->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration2);

      fieldDeclarations->add (variableName2, fieldDeclaration2);
    }
  }

  vector <string> planFunctionSizeVariables;

  planFunctionSizeVariables.push_back (pblkMapSize);
  planFunctionSizeVariables.push_back (pindOffsSize);
  planFunctionSizeVariables.push_back (pindSizesSize);
  planFunctionSizeVariables.push_back (pnelemsSize);
  planFunctionSizeVariables.push_back (pnthrcolSize);
  planFunctionSizeVariables.push_back (poffsetSize);
  planFunctionSizeVariables.push_back (pthrcolSize);

  for (vector <string>::iterator it = planFunctionSizeVariables.begin (); it
      != planFunctionSizeVariables.end (); ++it)
  {
    SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (*it,
        FortranTypesBuilder::getFourByteInteger (), NULL, moduleScope);

    fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    deviceDatatypeStatement->get_definition ()->append_member (fieldDeclaration);

    fieldDeclarations->add (*it, fieldDeclaration);
  }
}

FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop::FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranCUDAOpDatCardinalitiesDeclaration (parallelLoop, moduleScope)
{
  addIndirectLoopFields ();
}
