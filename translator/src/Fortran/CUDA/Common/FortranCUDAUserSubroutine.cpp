#include "FortranCUDAUserSubroutine.h"
#include "FortranParallelLoop.h"
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include <FortranCUDAConstantDeclarations.h>
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

  Debug::getInstance ()->debugMessage ("Outputting and modifying statements",
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
         * functions in the future (?). Probably not in OP2
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
      Debug::getInstance ()->debugMessage (
          "Appending (non-variable-declaration) statement",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      appendStatement (*it, subroutineScope);
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

        bool isFormalParamater = false;

        for (SgInitializedNamePtrList::iterator paramIt =
            originalParameters->get_args ().begin (); paramIt
            != originalParameters->get_args ().end (); ++paramIt, ++OP_DAT_ArgumentGroup)
        {
          string const formalParamterName =
              (*paramIt)->get_name ().getString ();

          if (iequals (variableName, formalParamterName))
          {
            isFormalParamater = true;

            if (parallelLoop->isIndirect (OP_DAT_ArgumentGroup)
                && parallelLoop->isRead (OP_DAT_ArgumentGroup))
            {
              Debug::getInstance ()->debugMessage ("'" + variableName
                  + "' is an INDIRECT formal parameter which is READ",
                  Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

              SgVariableDeclaration
                  * variableDeclaration =
                      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, type, subroutineScope,
                          formalParameters, 1, CUDA_SHARED);
                      
               ROSE_ASSERT ( variableDeclaration != NULL );
            }
            else if (parallelLoop->isGlobal (OP_DAT_ArgumentGroup)
                && !parallelLoop->isArray (OP_DAT_ArgumentGroup)
                && parallelLoop->isRead (OP_DAT_ArgumentGroup))
            {
              Debug::getInstance ()->debugMessage ("'" + variableName
                  + "' is a GLOBAL SCALAR formal parameter which is READ",
                  Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

              SgVariableDeclaration
                  * variableDeclaration =
                      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, type, subroutineScope,
                          formalParameters, 1, VALUE);
            }
            else
            {
              Debug::getInstance ()->debugMessage ("'" + variableName
                  + "' is a formal parameter "
                  + parallelLoop->getOpDatInformation (OP_DAT_ArgumentGroup),
                  Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

              SgVariableDeclaration
                  * variableDeclaration =
                      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, type, subroutineScope,
                          formalParameters, 1, CUDA_DEVICE);
            }
          }
        }

        if (isFormalParamater == false)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
              + "' is NOT a formal parameter", Debug::HIGHEST_DEBUG_LEVEL,
              __FILE__, __LINE__);

          SgVariableDeclaration
              * variableDeclaration =
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
  FortranCUDAConstantDeclarations * CUDAconstants)
{
  /*
   * ======================================================
   * First removes duplicates in calledRoutines
   * ======================================================
   */
  sort ( calledRoutines.begin(), calledRoutines.end() );
  calledRoutines.erase ( unique ( calledRoutines.begin(), calledRoutines.end() ), calledRoutines.end() );

  vector < SgProcedureHeaderStatement * > :: iterator it;
  for ( it = calledRoutines.begin(); it != calledRoutines.end(); it++ )
  {
   /*
    * ======================================================
    * Stil does not check if the routine is already present
    * ======================================================
    */
   
    Debug::getInstance ()->debugMessage ("Appending new subroutine '"
        + (*it)->get_name ().getString () + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
   
    FortranCUDAUserSubroutine * newRoutine = new FortranCUDAUserSubroutine ( moduleScope, 
        parallelLoop, declarations, (*it)->get_name ().getString () );
   
    additionalSubroutines.push_back (newRoutine);
  }
  
  vector < FortranUserSubroutine * > :: iterator itRecursive;
  for ( itRecursive = additionalSubroutines.begin(); itRecursive != additionalSubroutines.end(); itRecursive++ )
  {      
    FortranCUDAUserSubroutine * cudaSubroutineCasting = (FortranCUDAUserSubroutine *) *itRecursive;
    CUDAconstants->patchReferencesToCUDAConstants (
      (cudaSubroutineCasting )->getSubroutineHeaderStatement ());
          
    RoseHelper::forceOutputOfCodeToFile (
      (cudaSubroutineCasting )->getSubroutineHeaderStatement ());
  }

  for ( itRecursive = additionalSubroutines.begin(); itRecursive != additionalSubroutines.end(); itRecursive++ )
  {    
    FortranCUDAUserSubroutine * cudaSubroutineCasting = (FortranCUDAUserSubroutine *) *itRecursive;
    cudaSubroutineCasting->appendAdditionalSubroutines (moduleScope, parallelLoop, declarations, CUDAconstants);
  }  
}


FortranCUDAUserSubroutine::FortranCUDAUserSubroutine (
    SgScopeStatement * moduleScope, FortranParallelLoop * parallelLoop,
    FortranProgramDeclarationsAndDefinitions * declarations) :
  FortranUserSubroutine (moduleScope, parallelLoop, declarations)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaDevice ();

  createStatements (); 
}


FortranCUDAUserSubroutine::FortranCUDAUserSubroutine (
    SgScopeStatement * moduleScope, FortranParallelLoop * parallelLoop,
    FortranProgramDeclarationsAndDefinitions * declarations, string subroutineName) :
  FortranUserSubroutine (moduleScope, parallelLoop, declarations, subroutineName)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaDevice ();

  createStatements ();
}

