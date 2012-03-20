


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


#include "FortranCUDAUserSubroutine.h"
#include "FortranParallelLoop.h"
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include <FortranConstantDeclarations.h>
#include <RoseHelper.h>
#include "Debug.h"
#include "Exceptions.h"
#include <boost/algorithm/string/predicate.hpp>
#include <algorithm>

void
FortranCUDAUserSubroutine::createStatements ()
{
  using namespace SageInterface;
  using boost::iequals;
  using std::string;
  using std::vector;
  
  class TreeVisitor: public AstSimpleProcessing
  {
    private:
    /*
     * ======================================================
     * The recursive visit of a user subroutine populates
     * this vector with successive function calls which are
     * then appended after the visit
     * ======================================================
     */            
    vector < SgProcedureHeaderStatement * > calledRoutines;

    public:

      vector < SgProcedureHeaderStatement * > getCalledRoutinesInStatement()
      {
        return calledRoutines;
      }
      
      TreeVisitor ()
      {
      }

      virtual void
      visit (SgNode * node)
      {
        SgExprStatement * isExprStatement = isSgExprStatement ( node );
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
             * functions in the future (?). Probably not in OP2
             * ======================================================
             */
            SgProcedureHeaderStatement * isProcedureHeaderStatement = isSgProcedureHeaderStatement ( 
              functionCallExp->getAssociatedFunctionDeclaration() );

            calledRoutines.push_back ( isProcedureHeaderStatement );
          }
        }
      }
  };
  
  Debug::getInstance ()->debugMessage ("User subroutine: outputting and modifying statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgFunctionParameterList * originalParameters =
      originalSubroutine->get_parameterList ();

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
         * ======================================================
         */            
        SgProcedureHeaderStatement * isProcedureHeaderStatement = isSgProcedureHeaderStatement ( 
          functionCallExp->getAssociatedFunctionDeclaration() );
            
        calledRoutines.push_back ( isProcedureHeaderStatement );
      }
    }

    SgVariableDeclaration * isVariableDeclaration = isSgVariableDeclaration (
        *it);

    if (isVariableDeclaration == NULL)
    { 
      /*
       * ======================================================
       * Do not append use statement, because other subroutines
       * are directly appended to the CUDA module
       * ======================================================
       */                  
      SgUseStatement * isUseStmt = isSgUseStatement ( *it );
      if (isUseStmt != NULL)
        {
          Debug::getInstance ()->debugMessage (
                "Not appending use statement",
                Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);
        }
      else
      {
        Debug::getInstance ()->debugMessage (
              "Appending (non-variable-declaration) statement",
              Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        appendStatement (*it, subroutineScope);

        /*
         * ======================================================
         * Recursively look for subroutine calls inside shallow
         * nodes in the routines (e.g. when a call is inside an 
         * if). After the visit get the generated vector of names
         * and append it to the userSubroutine vector
         * ======================================================
         */                  
        TreeVisitor * visitor = new TreeVisitor ();
	  
        visitor->traverse (*it, preorder);
          
        Debug::getInstance ()->debugMessage ("Appending deep subroutine calls", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

          
        vector < SgProcedureHeaderStatement * > deepStatementCalls = visitor->getCalledRoutinesInStatement ();
        vector < SgProcedureHeaderStatement * >::iterator itDeepCalls;
        for (itDeepCalls = deepStatementCalls.begin(); itDeepCalls != deepStatementCalls.end(); ++itDeepCalls)
          calledRoutines.push_back (*itDeepCalls);
      
        Debug::getInstance ()->debugMessage ("Appending deep subroutine calls", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
      }
    }
    else
    {
      Debug::getInstance ()->debugMessage ("Appending variable declaration",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      unsigned int OP_DAT_ArgumentGroup = 1;

      for (SgInitializedNamePtrList::iterator variableIt =
          isVariableDeclaration->get_variables ().begin (); variableIt
          != isVariableDeclaration->get_variables ().end (); ++variableIt)
      {
        string const variableName = (*variableIt)->get_name ().getString ();

        SgType * type = (*variableIt)->get_typeptr ();

        /*
         * ======================================================
         * Specification of "value" attribute is only
         * for user kernels. Our call convention is that
         * in all deeper level calls we always pass parameters
         * by reference (see else branch below)
         * ======================================================
         */                  
        
        bool isFormalParamater = false;
	
        for (SgInitializedNamePtrList::iterator paramIt =
            originalParameters->get_args ().begin (); paramIt
            != originalParameters->get_args ().end (); ++paramIt, ++OP_DAT_ArgumentGroup)
        {
          string const formalParamterName = (*paramIt)->get_name ().getString ();

          if (iequals (variableName, formalParamterName))
          {
            isFormalParamater = true;

            if (parallelLoop->isIndirect (OP_DAT_ArgumentGroup)
                && parallelLoop->isRead (OP_DAT_ArgumentGroup))
            {
              Debug::getInstance ()->debugMessage ("'" + variableName
                  + "' is an INDIRECT formal parameter which is READ",
                  Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

              SgVariableDeclaration * variableDeclaration;
              if ( isUserKernel == true )
                variableDeclaration =
                      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, type, subroutineScope,
                          formalParameters, 0); //, CUDA_SHARED);
              else
                  variableDeclaration =
                      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, type, subroutineScope,
                          formalParameters, 0);

                ROSE_ASSERT ( variableDeclaration != NULL );
            }
            else if (parallelLoop->isGlobal (OP_DAT_ArgumentGroup)
                     && !parallelLoop->isArray (OP_DAT_ArgumentGroup)
                     && parallelLoop->isRead (OP_DAT_ArgumentGroup))
            {
              Debug::getInstance ()->debugMessage ("'" + variableName
                + "' is a GLOBAL SCALAR formal parameter which is READ",
                Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

              if ( isUserKernel == true )
                SgVariableDeclaration * variableDeclaration =
                  FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName, type, subroutineScope, formalParameters, 1, VALUE);
              else
                SgVariableDeclaration * variableDeclaration =
                  FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName, type, subroutineScope, formalParameters, 0);
            }
            else
            {
              Debug::getInstance ()->debugMessage ("'" + variableName
                  + "' is a formal parameter "
                  + parallelLoop->getOpDatInformation (OP_DAT_ArgumentGroup),
                  Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

              if ( isUserKernel == true )
                SgVariableDeclaration * variableDeclaration =
                  FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName, type, subroutineScope, formalParameters, 0); //, CUDA_DEVICE);
              else
                SgVariableDeclaration * variableDeclaration =
                  FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName, type, subroutineScope, formalParameters, 0);
            }
          }
        }
          
        if (isFormalParamater == false)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
            + "' is NOT a formal parameter", Debug::HIGHEST_DEBUG_LEVEL,
            __FILE__, __LINE__);

          SgVariableDeclaration * variableDeclaration =
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, type, subroutineScope);
        }
      }
    }
  }
}

void
FortranCUDAUserSubroutine::createLocalVariableDeclarations ()
{
}

void
FortranCUDAUserSubroutine::createFormalParameterDeclarations ()
{
}


/*
 * ======================================================
 * This function appends all additional subroutines
 * called inside the user subroutine. It is specialised
 * for CUDA in the related subclass
 * ======================================================
 */
void FortranCUDAUserSubroutine::appendAdditionalSubroutines ( SgScopeStatement * moduleScope,
  FortranParallelLoop * parallelLoop, FortranProgramDeclarationsAndDefinitions * declarations,
  FortranConstantDeclarations * CUDAconstants, std::vector < SgProcedureHeaderStatement * > * allCalledRoutines)
{
  using std::vector;
  using boost::iequals;
  /*
   * ======================================================
   * First removes duplicates in calledRoutines itself
   * ======================================================
   */
  sort ( calledRoutines.begin(), calledRoutines.end() );
  calledRoutines.erase ( unique ( calledRoutines.begin(), calledRoutines.end() ), calledRoutines.end() );

  Debug::getInstance ()->debugMessage ("Before removing, the list of routine calls found in the user kernels is: ",
   Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  vector < SgProcedureHeaderStatement * > :: iterator routinesIt2;
  for ( routinesIt2 = calledRoutines.begin (); routinesIt2 != calledRoutines.end (); routinesIt2++ )
  {
    string appendingSubroutine = (*routinesIt2)->get_name ().getString ();
    Debug::getInstance ()->debugMessage (appendingSubroutine,
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  }
  
  /*
   * ======================================================
   * The removes routines already appended by other user
   * kernels, using the list in allCalledRoutines
   * ======================================================
   */
  Debug::getInstance ()->debugMessage ("Removing global duplicates, the number of routines in the list is: '"
    + boost::lexical_cast<string> ((int) calledRoutines.size()) + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector < SgProcedureHeaderStatement * > :: iterator routinesIt;
  for ( routinesIt = calledRoutines.begin (); routinesIt != calledRoutines.end (); ) //routinesIt++ )
  {
    string appendingSubroutine = (*routinesIt)->get_name ().getString ();

    Debug::getInstance ()->debugMessage ("Checking routine for deletion: '"
      + appendingSubroutine + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    bool foundAndErased = false;
    vector < SgProcedureHeaderStatement * > :: iterator finder;
    for ( finder = allCalledRoutines->begin (); finder != allCalledRoutines->end (); finder++ )
    {
      Debug::getInstance ()->debugMessage ("Checking against: '"
        + (*finder)->get_name ().getString () + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

      if ( iequals ((*finder)->get_name ().getString (), appendingSubroutine) )
      {
        /*
         * ======================================================
         * Routine already appended by another user kernel:
         * delete it from list of routines to be appended for
         * this user kernel, and exit this loop
         * ======================================================
         */      
        Debug::getInstance ()->debugMessage ("Deleting: '"
          + appendingSubroutine + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
        
        calledRoutines.erase (routinesIt++);

        if ( calledRoutines.empty () ) return;

        foundAndErased = true;

        routinesIt--;
        break;
      }      
    }

    if ( foundAndErased == false )
    {
        /*
         * ======================================================
         * New routine: it must be added to the list of 
         * routines called by all previous user kernels because
         * recursively called routines need to discard those
         * already appended by this routine
         * ======================================================
         */
         Debug::getInstance ()->debugMessage ("Not found, appending: '"
          + appendingSubroutine + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
        
        allCalledRoutines->push_back ( *routinesIt );
        
        routinesIt++;
    }
  }
  
  
  vector < SgProcedureHeaderStatement * > :: iterator it;
  for ( it = calledRoutines.begin(); it != calledRoutines.end(); it++ )
  {
   
    string calledSubroutineName = (*it)->get_name ().getString ();
    
    Debug::getInstance ()->debugMessage ("Appending new subroutine '"
        + calledSubroutineName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
   
    FortranCUDAUserSubroutine * newRoutine = new FortranCUDAUserSubroutine ( moduleScope, 
        parallelLoop, declarations, calledSubroutineName );
   
    additionalSubroutines.push_back (newRoutine);        
  }
  
  vector < FortranUserSubroutine * > :: iterator itRecursive;
  for ( itRecursive = additionalSubroutines.begin(); itRecursive != additionalSubroutines.end(); itRecursive++ )
  {
    FortranCUDAUserSubroutine * cudaSubroutineCasting = (FortranCUDAUserSubroutine *) *itRecursive;

    CUDAconstants->patchReferencesToConstants (
      (cudaSubroutineCasting )->getSubroutineHeaderStatement ());
          
    RoseHelper::forceOutputOfCodeToFile (
      (cudaSubroutineCasting )->getSubroutineHeaderStatement ());
  }

  for ( itRecursive = additionalSubroutines.begin(); itRecursive != additionalSubroutines.end(); itRecursive++ )
  {
    FortranCUDAUserSubroutine * cudaSubroutineCasting = (FortranCUDAUserSubroutine *) *itRecursive;

    cudaSubroutineCasting->appendAdditionalSubroutines (moduleScope, parallelLoop, declarations, CUDAconstants, allCalledRoutines);
  }
}

/*
 * ======================================================
 * This contructor is for user kernels only. Use the
 * next one (with the subroutineName specified) to create
 * a deeper call level user subroutine (i.e. user 
 * subroutines called inside some level of call graph of
 * a user kernel)
 * ======================================================
 */
 
FortranCUDAUserSubroutine::FortranCUDAUserSubroutine (
    SgScopeStatement * moduleScope, FortranParallelLoop * parallelLoop,
    FortranProgramDeclarationsAndDefinitions * declarations) :
  FortranUserSubroutine (moduleScope, parallelLoop, declarations)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaDevice ();

//  createStatements ();
}

/*
 * ======================================================
 * This contructor is for non user kernels only (see 
 * above)
 * ======================================================
 */

FortranCUDAUserSubroutine::FortranCUDAUserSubroutine (
    SgScopeStatement * moduleScope, FortranParallelLoop * parallelLoop,
    FortranProgramDeclarationsAndDefinitions * declarations, string subroutineName):
  FortranUserSubroutine (moduleScope, parallelLoop, declarations, subroutineName)  
{
  subroutineHeaderStatement->get_functionModifier ().setCudaDevice ();

//  createStatements ();
}

