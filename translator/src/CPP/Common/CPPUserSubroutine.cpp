


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


#include "CPPUserSubroutine.h"
#include "CPPParallelLoop.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "OP2Definitions.h"
#include "RoseHelper.h"

void
CPPUserSubroutine::visit (SgNode * node)
{
  using std::string;
  using std::find;

  SgVarRefExp * variableReference = isSgVarRefExp (node);

  if (variableReference != NULL)
  {
    string const variableName = variableReference->get_symbol ()->get_name ();

    if (declarations->isOpConstDefinition (variableName))
    {
      Debug::getInstance ()->debugMessage ("Found reference to OP_DECL_CONST '"
          + variableName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      if (find (referencedOpDeclConsts.begin (), referencedOpDeclConsts.end (),
          variableName) == referencedOpDeclConsts.end ())
      {
        referencedOpDeclConsts.push_back (variableName);
      }
    }
  }
}

void
CPPUserSubroutine::createStatements ()
{
  using namespace SageInterface;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Outputting and modifying statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <SgStatement *> originalStatements =
      originalSubroutine->get_definition ()->get_body ()->get_statements ();

  for (vector <SgStatement *>::iterator it = originalStatements.begin (); it
      != originalStatements.end (); ++it)
  { 
    appendStatement (*it, subroutineScope);
  }
}

void
CPPUserSubroutine::createLocalVariableDeclarations ()
{
}

void
CPPUserSubroutine::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Outputting formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgFunctionParameterList * originalParameters =
      originalSubroutine->get_parameterList ();

  int opDatIndex = 1;

  for (SgInitializedNamePtrList::iterator paramIt =
      originalParameters->get_args ().begin (); paramIt
      != originalParameters->get_args ().end (); ++paramIt)
  {
    string const variableName = (*paramIt)->get_name ().getString ();

    SgType * type = (*paramIt)->get_type ();

    SgVariableDeclaration
        * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, type, subroutineScope, formalParameters);


    if (parallelLoop->isDirectLoop ()) 
    {
      //if (!parallelLoop->isDirect (opDatIndex)) 
      //{
        int dimension = parallelLoop->getOpDatDimension (opDatIndex);

        if (!parallelLoop->isIncremented (opDatIndex))
        {
          if (dimension == 1)
          {
            (*variableDeclaration->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();
          } else
          {
            //(*variableDeclaration->get_variables ().begin ())->get_storageModifier ().setOpenclLocal ();
          }
        }
      //}
    } else 
    {
      if (!parallelLoop->isDirect (opDatIndex))
      {
        if (!parallelLoop->isIncremented (opDatIndex)) 
        {
          (*variableDeclaration->get_variables ().begin ())->get_storageModifier ().setOpenclLocal ();
        }
      } else
      {
        if (parallelLoop->isIncremented (opDatIndex)) 
        {
          (*variableDeclaration->get_variables ().begin ())->get_storageModifier ().setOpenclLocal ();
        } else
        {
          (*variableDeclaration->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();
        } 
      }      
    }
    //(*variableDeclaration->get_variables ().begin())->get_storageModifier ().setOpenclLocal ();
    ++opDatIndex;
  }

/*
 * Nicolas: unused by OPEN, CUDA ? is that needed for OpenCL ?
 * Roxana: this is needed for the current OpenCL implementation. 
*/ 
  for (vector <string>::const_iterator it = referencedOpDeclConsts.begin (); it
      != referencedOpDeclConsts.end (); ++it)
  {
    OpConstDefinition * opConst = declarations->getOpConstDefinition (*it);

    SgType * type = opConst->getType ();

    string variableName = opConst->getVariableName ();

    if (variableName.compare("qinf") == 0 || variableName.compare("*qinf") == 0) {
      type = buildOpaqueType("float ", subroutineScope);  
    } 

    SgVariableDeclaration
        * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                *it, buildPointerType (type), subroutineScope, formalParameters);


    //TODO: let's not write architecture dependent stuff in the CPP COmmon library! 
    //But not sure how...
    (*variableDeclaration->get_variables ().begin())->get_storageModifier ().setOpenclConstant ();
  }
/* */
}

CPPUserSubroutine::CPPUserSubroutine (SgScopeStatement * moduleScope,
    CPPParallelLoop * parallelLoop,
    CPPProgramDeclarationsAndDefinitions * declarations) :
      UserSubroutine <SgFunctionDeclaration,
          CPPProgramDeclarationsAndDefinitions> (parallelLoop, declarations)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  originalSubroutine = declarations->getSubroutine (
      parallelLoop->getUserSubroutineName ());

  traverse (originalSubroutine, preorder);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();

  RoseHelper::forceOutputOfCodeToFile (subroutineHeaderStatement);
}
