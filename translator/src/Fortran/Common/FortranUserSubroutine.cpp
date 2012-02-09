


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


#include "FortranUserSubroutine.h"
#include "FortranParallelLoop.h"
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "RoseStatementsAndExpressionsBuilder.h"

void
FortranUserSubroutine::createStatements ()
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
        SgExprStatement * isExprStatement = isSgExprStatement ( *it );
    if ( isExprStatement != NULL )
    {      
      SgFunctionCallExp * functionCallExp = isSgFunctionCallExp ( isExprStatement->get_expression() );
    
      if ( functionCallExp != NULL )
      {
        string const
            calleeName =
                functionCallExp->getAssociatedFunctionSymbol ()->get_name ().getString ();

        Debug::getInstance ()->debugMessage ("Found function call in user subroutine "
            + calleeName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        /*
         * ======================================================
         * As we are in fortran, all user subroutines must be
         * SgProcedureHeaderStatements = subroutines and not
         * functions. This might be extended to cover also 
         * functions in the future (probably not in OP2)
         * Warning: only shallow calls (and not ones nested 
         * in some sub-block of the subroutines) are called, for
         * debugging the CUDA version in which we are forced
         * to copy-and-paste subroutines in the generated module
         * ======================================================
         */            
        SgProcedureHeaderStatement * isProcedureHeaderStatement = isSgProcedureHeaderStatement ( 
          functionCallExp->getAssociatedFunctionDeclaration() );
            
        calledRoutines.push_back ( isProcedureHeaderStatement );
      }
    }

    SgVariableDeclaration * isVariableDeclaration = isSgVariableDeclaration (
       *it);
    

    appendStatement (*it, subroutineScope);
  }
}

void
FortranUserSubroutine::createLocalVariableDeclarations ()
{

}

void
FortranUserSubroutine::createFormalParameterDeclarations ()
{
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Outputting formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgFunctionParameterList * originalParameters =
      originalSubroutine->get_parameterList ();

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
  }
}

/*
 * ======================================================
 * This function appends all additional subroutines
 * called inside the user subroutine. It is specialised
 * for CUDA in the related subclass
 * ======================================================
 */
void FortranUserSubroutine::appendAdditionalSubroutines ( SgScopeStatement * moduleScope,
  FortranParallelLoop * parallelLoop, FortranProgramDeclarationsAndDefinitions * declarations,
  std::vector < SgProcedureHeaderStatement * > * allCalledRoutines)
{
  using std::vector;
  /*
   * ======================================================
   * First removes duplicates in calledRoutines
   * Warning: does not avoid duplicates for now!!!!!
   * ======================================================
   */
  sort ( calledRoutines.begin(), calledRoutines.end() );
  calledRoutines.erase ( unique ( calledRoutines.begin(), calledRoutines.end() ), calledRoutines.end() );
  
  
  vector < SgProcedureHeaderStatement * > :: iterator it;
  for ( it = calledRoutines.begin(); it != calledRoutines.end(); it++ )
  {
  
   Debug::getInstance ()->debugMessage ("Appending new subroutine '"
        + (*it)->get_name ().getString () + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

   
    FortranUserSubroutine * newRoutine = new FortranUserSubroutine ( moduleScope, parallelLoop, declarations, (*it)->get_name ().getString () );
   
    /*
     * ======================================================
     * Unlike CUDA, we need here to create the statements
     * ======================================================
     */
    newRoutine->createStatements();
    
    additionalSubroutines.push_back (newRoutine);
  }
}

FortranUserSubroutine::FortranUserSubroutine (SgScopeStatement * moduleScope,
    FortranParallelLoop * parallelLoop,
    FortranProgramDeclarationsAndDefinitions * declarations) :
  UserSubroutine <SgProcedureHeaderStatement,
      FortranProgramDeclarationsAndDefinitions> (parallelLoop, declarations)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  originalSubroutine = declarations->getSubroutine (
      parallelLoop->getUserSubroutineName ());
}


FortranUserSubroutine::FortranUserSubroutine (SgScopeStatement * moduleScope,
    FortranParallelLoop * parallelLoop,
    FortranProgramDeclarationsAndDefinitions * declarations,
    string subroutineName) :
  UserSubroutine <SgProcedureHeaderStatement,
      FortranProgramDeclarationsAndDefinitions> (parallelLoop, declarations, subroutineName)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  originalSubroutine = declarations->getSubroutine (
      subroutineName);
}

