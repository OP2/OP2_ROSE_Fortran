


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


#include <OP2Definitions.h>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <FortranProgramDeclarationsAndDefinitions.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <CompilerGeneratedNames.h>
#include <FortranTypesBuilder.h>

void
FortranCUDAInitialiseConstantsSubroutine::createStatements ()
{
  using namespace SageInterface;
  using namespace SageBuilder;
  using namespace LoopVariableNames;

  /*
   * ======================================================
   * Create a list of assignment from constant names
   * to cuda constant names. Handle array assignment as
   * do-loops
   * ======================================================
   */

  Debug::getInstance ()->debugMessage ("Appending assignments for CUDA constants initialisation",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (std::map <std::string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    std::string const & variableName = it->first;

    OpConstDefinition * constDefinition = it->second;

    SgType * type = constDefinition->getType ();
 
    if ( isSgArrayType (type) != NULL )
      {
	SgBasicBlock * initLoopBody = buildBasicBlock ();

	SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          constantDeclarations->getReference (oldNamesToNewNames[variableName]),
	  variableDeclarations->getReference (getIterationCounterVariableName (1)));

	SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
          variableDeclarations->getReference (variableName),
	  variableDeclarations->getReference (getIterationCounterVariableName (1)));

	SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression1, arrayExpression2);

	appendStatement (assignmentStatement, initLoopBody);

	SgAssignOp * initLoopLowerBoundExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), buildIntVal (1));	

	SgFortranDo * initialisationLoop =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
	      initLoopLowerBoundExpression, buildIntVal(constDefinition->getDimension()),
              buildIntVal (1), initLoopBody);

	appendStatement (initialisationLoop, subroutineScope);
      } 
    else
      {
	SgExprStatement * assignmentStatement = buildAssignStatement (
          constantDeclarations->getReference ( oldNamesToNewNames[it->first] ),
	  variableDeclarations->getReference ( it->first ));

	appendStatement (assignmentStatement, subroutineScope);
      }
  }
}

void
FortranCUDAInitialiseConstantsSubroutine::createLocalVariableDeclarations ()
{
  
}

void
FortranCUDAInitialiseConstantsSubroutine::createFormalParameterDeclarations ()
{
  using std::vector;
  using std::map;
  using std::string;
  using namespace SageInterface;
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  
  Debug::getInstance ()->debugMessage ("Outputting formal parameters for CUDA constant initialisation routine",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

      
  for (std::map <std::string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    std::string const & variableName = it->first;

    OpConstDefinition * constDefinition = it->second;

    SgType * type = constDefinition->getType ();
    
    variableDeclarations->add (
      variableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
        variableName, type, subroutineScope, formalParameters));

  }

  /*
   * ======================================================
   * Append iteration variable for initialising 
   * constant arrays
   * ======================================================
   */
  variableDeclarations ->add (getIterationCounterVariableName (1),
    FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
        getIterationCounterVariableName (1),
        FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}



FortranCUDAInitialiseConstantsSubroutine::FortranCUDAInitialiseConstantsSubroutine (std::string subroutineName, SgScopeStatement * moduleScope,
  FortranProgramDeclarationsAndDefinitions * allDeclarations, std::map <std::string, std::string> _oldNamesToNewNames,
  ScopedVariableDeclarations * _constantDeclarations):
  Subroutine <SgProcedureHeaderStatement> (subroutineName), declarations(allDeclarations), oldNamesToNewNames(_oldNamesToNewNames),
  constantDeclarations(_constantDeclarations)
{
  using namespace SageInterface;
  using namespace SageBuilder;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  subroutineHeaderStatement->get_functionModifier ().setCudaHost ();
      
  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  appendStatement (
      FortranStatementsAndExpressionsBuilder::buildImplicitNoneStatement (),
      subroutineScope);

  createFormalParameterDeclarations ();
  
  createStatements (); 
}
