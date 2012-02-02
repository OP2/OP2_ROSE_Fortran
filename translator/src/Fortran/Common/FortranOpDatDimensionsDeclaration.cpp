


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


#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranParallelLoop.h>
#include <ScopedVariableDeclarations.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CompilerGeneratedNames.h>
#include <Debug.h>
#include <boost/lexical_cast.hpp>
#include <rose.h>

void
FortranOpDatDimensionsDeclaration::addFields ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  fieldDeclarations = new ScopedVariableDeclarations ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDirect (i) || parallelLoop->isIndirect (i))
    {
      Debug::getInstance ()->debugMessage (
          "Adding dimensions field for OP_DAT " + lexical_cast <string> (i),
          Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      string const & variableName = getOpDatDimensionName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      typeStatement->get_definition ()->append_member (fieldDeclaration);

      fieldDeclarations->add (variableName, fieldDeclaration);
    }
  }
}

void
FortranOpDatDimensionsDeclaration::addTypeDeclaration ()
{
  using namespace SageInterface;

  typeStatement = RoseStatementsAndExpressionsBuilder::buildTypeDeclaration (
      parallelLoop->getUserSubroutineName () + "_opDatDimensions", moduleScope);

  typeStatement->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (typeStatement, moduleScope);
}

SgClassType *
FortranOpDatDimensionsDeclaration::getType ()
{
  return typeStatement->get_type ();
}

SgVarRefExp *
FortranOpDatDimensionsDeclaration::getOpDatDimensionField (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace OP2VariableNames;

  return fieldDeclarations->getReference (getOpDatDimensionName (
      OP_DAT_ArgumentGroup));
}

FortranOpDatDimensionsDeclaration::FortranOpDatDimensionsDeclaration (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  parallelLoop (parallelLoop), moduleScope (moduleScope)
{
  Debug::getInstance ()->debugMessage (
      "Creating OP_DAT dimensions type declaration", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  addTypeDeclaration ();

  addFields ();
}
