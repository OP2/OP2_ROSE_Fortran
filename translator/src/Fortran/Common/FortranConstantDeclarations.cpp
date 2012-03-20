
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


#include "FortranConstantDeclarations.h"
#include <FortranInitialiseConstantsSubroutine.h>
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "OP2Definitions.h"
#include "ScopedVariableDeclarations.h"
#include "Debug.h"
#include <rose.h>
#include "../../../../../ROSE/rose-0.9.5a-15165_build/src/frontend/SageIII/Cxx_Grammar.h"

SgVarRefExp *
FortranConstantDeclarations::getReferenceToNewVariable (
    std::string const & originalName)
{
  return variableDeclarations->getReference (getNewConstantVariableName (
      originalName));
}

bool
FortranConstantDeclarations::isOP2Constant (
    std::string const & originalName)
{
  return oldNamesToNewNames.count (originalName) != 0;
}

std::string
FortranConstantDeclarations::getNewConstantVariableName (
    std::string const & originalName)
{
  return originalName + "_OP2_CONSTANT";
}

void
FortranConstantDeclarations::addDeclarations (
    FortranProgramDeclarationsAndDefinitions * declarations,
    SgScopeStatement * moduleScope, bool isCUDA)
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
    string const & variableName = it->first;

    OpConstDefinition * constDefinition = it->second;

    SgType * type = constDefinition->getType ();

    Debug::getInstance ()->debugMessage ("Analysing OP_DECL_CONST with name '"
        + variableName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    string const & newVariableName = getNewConstantVariableName (variableName);

    oldNamesToNewNames[variableName] = newVariableName;
    
    if ( constDefinition->getDimension () == 1 )
      if ( isCUDA == true )
        variableDeclarations->add (newVariableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              newVariableName, type, moduleScope, 1, CUDA_CONSTANT));
      else
        variableDeclarations->add (newVariableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              newVariableName, type, moduleScope, 0));
    else
      /*
       * ======================================================
       * Constant arrays are mapped to device memory,
       * currently irregardeless of their size. In the 
       * future, dimension check might be used to decide
       * the memory mapping
       * ======================================================
       */
       if ( isCUDA == true )
        variableDeclarations->add (newVariableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              newVariableName, type, moduleScope, 1, CUDA_DEVICE));
       else
        variableDeclarations->add (newVariableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              newVariableName, type, moduleScope, 0));         
  }
}

void
FortranConstantDeclarations::patchReferencesToConstants (
    SgProcedureHeaderStatement * procedureHeader)
{
  Debug::getInstance ()->debugMessage ("Patching references to constants",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  class TreeVisitor: public AstSimpleProcessing
  {
    private:

      FortranConstantDeclarations * OP2Constants;

    public:

      TreeVisitor (FortranConstantDeclarations * OP2Constants) :
        OP2Constants (OP2Constants)
      {
      }

      virtual void
      visit (SgNode * node)
      {
        using std::string;

        SgVarRefExp * variableReference = isSgVarRefExp (node);

        if (variableReference != NULL)
        {
          string const variableName =
              variableReference->get_symbol ()->get_name ();

          if (OP2Constants->isOP2Constant (variableName))
          {
            SgVarRefExp * newReference =
                OP2Constants->getReferenceToNewVariable (variableName);

            variableReference->set_symbol (newReference->get_symbol ());
          }
        }
      }
  };

  TreeVisitor * visitor = new TreeVisitor (this);

  visitor->traverse (procedureHeader, preorder);
}

void
FortranConstantDeclarations::appendConstantInitialisationToModule ( SgScopeStatement * moduleScope,
    FortranProgramDeclarationsAndDefinitions * declarations, bool isCuda)
{

  /*
   * ======================================================
   * This function appends a new subroutine to the CUDA
   * module which initialises OP2 constants to their
   * respective values. Its call is appended just after
   * the last call to op_decl_const in the user code
   * ======================================================
   */
  
  std::string subroutineName = "initOP2Constants";
  
  initialisationRoutine = new FortranInitialiseConstantsSubroutine (subroutineName, moduleScope,
      declarations, oldNamesToNewNames, variableDeclarations, isCuda);
}

FortranConstantDeclarations::FortranConstantDeclarations (
    FortranProgramDeclarationsAndDefinitions * declarations,
    SgScopeStatement * moduleScope, bool isCUDA)
{
  variableDeclarations = new ScopedVariableDeclarations ();

  addDeclarations (declarations, moduleScope, isCUDA);
}
