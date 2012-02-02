


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


#include "FortranCUDAOpDatCardinalitiesDeclaration.h"
#include "FortranParallelLoop.h"
#include "FortranTypesBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "ScopedVariableDeclarations.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include <boost/lexical_cast.hpp>
#include <rose.h>

void
FortranCUDAOpDatCardinalitiesDeclaration::addFields ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isCardinalityDeclarationNeeded (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating cardinality field for OP_DAT "
                + lexical_cast <string> (i), Debug::HIGHEST_DEBUG_LEVEL,
            __FILE__, __LINE__);

        string const & variableName = getOpDatCardinalityName (i);

        SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
            variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
            moduleScope);

        fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

        deviceDatatypeStatement->get_definition ()->append_member (
            fieldDeclaration);

        fieldDeclarations->add (variableName, fieldDeclaration);
      }
    }
  }
}

SgClassType *
FortranCUDAOpDatCardinalitiesDeclaration::getType ()
{
  return deviceDatatypeStatement->get_type ();
}

ScopedVariableDeclarations *
FortranCUDAOpDatCardinalitiesDeclaration::getFieldDeclarations ()
{
  return fieldDeclarations;
}

FortranCUDAOpDatCardinalitiesDeclaration::FortranCUDAOpDatCardinalitiesDeclaration (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  parallelLoop (parallelLoop), moduleScope (moduleScope)
{
  using namespace SageInterface;

  fieldDeclarations = new ScopedVariableDeclarations ();

  deviceDatatypeStatement
      = RoseStatementsAndExpressionsBuilder::buildTypeDeclaration (
          parallelLoop->getUserSubroutineName () + "_opDatCardinalities",
          moduleScope);

  deviceDatatypeStatement->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (deviceDatatypeStatement, moduleScope);

  addFields ();
}
