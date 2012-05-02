


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


#include "FortranCUDAKernelSubroutine.h"
#include "FortranCUDAUserSubroutine.h"
#include "FortranParallelLoop.h"
#include "FortranCUDAModuleDeclarations.h"
#include "FortranOpDatDimensionsDeclaration.h"
#include "FortranReductionSubroutines.h"
#include "FortranCUDAReductionSubroutine.h"
#include "FortranTypesBuilder.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "RoseHelper.h"
#include "Reduction.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "CUDA.h"

void
FortranCUDAKernelSubroutine::createReductionLocalVariableInitialisation ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace ReductionVariableNames;
  using namespace PlanFunctionVariableNames;
  
  Debug::getInstance ()->debugMessage ("Adding initialisation of local thread variables for reductions",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i) && parallelLoop->isArray (i) )
    {
      SgExprStatement * initialiseStatement = NULL;
      
      if ( parallelLoop->isMaximised (i) || parallelLoop->isMinimised (i) )
      {

        SgExpression * partitionDisplacementDirectOrIndirect =
          buildSubtractOp (CUDA::getBlockId (BLOCK_X, subroutineScope), buildIntVal (1));

        SgMultiplyOp * partitionDisplacement = buildMultiplyOp (partitionDisplacementDirectOrIndirect, 
          buildIntVal( parallelLoop->getOpDatDimension(i)));

        SgSubtractOp * subtractExpression2 = buildSubtractOp (CUDA::getBlockId (
            BLOCK_X, subroutineScope), buildIntVal (1));

        SgAddOp * addExpression1 = buildAddOp (
          buildAddOp (variableDeclarations->getReference (getIterationCounterVariableName (1)), buildIntVal(1)),
          partitionDisplacement);
            
        SgAddOp * addExpression2 = buildAddOp (variableDeclarations->getReference (
          getIterationCounterVariableName (1)), subtractExpression2);

        SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (getReductionArrayDeviceName (i)),
            addExpression1);
                
        initialiseStatement = buildAssignStatement (
          buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatLocalName (i)),
            variableDeclarations->getReference (getIterationCounterVariableName (1))),
          parameterExpression1);
      }
      else
      {
        initialiseStatement = buildAssignStatement (
          buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatLocalName (i)),
            variableDeclarations->getReference (getIterationCounterVariableName (1))),
          buildFloatVal (0));
      }
      
      ROSE_ASSERT ( initialiseStatement != NULL );
            
      SgBasicBlock * initLoopBody = buildBasicBlock (initialiseStatement);

      SgAssignOp * loopInitialiserExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), buildIntVal (0));

      SgExpression * upperBoundExpression = buildSubtractOp (
        buildIntVal (parallelLoop->getOpDatDimension (i)), buildIntVal (1));

      SgFortranDo * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopInitialiserExpression, upperBoundExpression,
              buildIntVal (1), initLoopBody);
      
      appendStatement (loopStatement, subroutineScope);

    }
  }
  
}

void
FortranCUDAKernelSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace ReductionVariableNames;
  using namespace PlanFunctionVariableNames;
  
  using std::string;

  Debug::getInstance ()->debugMessage ("Adding reduction subroutine call",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      /*
       * ======================================================
       * Reduction operation parameter
       * ======================================================
       */

      SgIntVal * reductionType;

      if (parallelLoop->isIncremented (i))
      {
        reductionType = buildIntVal (INCREMENT);
      }
      else if (parallelLoop->isMaximised (i))
      {
        reductionType = buildIntVal (MAXIMUM);
      }
      else if (parallelLoop->isMinimised (i))
      {
        reductionType = buildIntVal (MINIMUM);
      }

      ROSE_ASSERT (reductionType != NULL);

      /*
       * ======================================================
       * Create reduction function call. Distinguishes between
       * scalar and array global variables, and dim = 1 arrays
       * (...due to fortran cuda...)
       * ======================================================
       */

      if ( parallelLoop->isArray (i) && parallelLoop->getOpDatDimension (i) > 1 )
      {

        /*
         * ======================================================
         * Index into the reduction array on the device:
         * For direct loops it is simply the blockidx%x because
         * I run only 1 grid for the loop
         * For indirect loops I need also the blockOffset,
         * because I run multiple grids, each for a partitions
         * colour
         * ======================================================
         */

        SgExpression * partitionDisplacementDirectOrIndirect = NULL;
        

        partitionDisplacementDirectOrIndirect = buildSubtractOp (CUDA::getBlockId (
            BLOCK_X, subroutineScope), buildIntVal (1));
        
        SgMultiplyOp * partitionDisplacement = buildMultiplyOp (partitionDisplacementDirectOrIndirect, 
          buildIntVal( parallelLoop->getOpDatDimension(i)));

        SgSubtractOp * subtractExpression2 = buildSubtractOp (CUDA::getBlockId (
            BLOCK_X, subroutineScope), buildIntVal (1));

        SgAddOp * addExpression1 = buildAddOp (
          buildAddOp (variableDeclarations->getReference (getIterationCounterVariableName (1)), buildIntVal(1)),
          partitionDisplacement);
            
        SgAddOp * addExpression2 = buildAddOp (variableDeclarations->getReference (
          getIterationCounterVariableName (1)), subtractExpression2);

        SgSubscriptExpression * subscriptExpressionReductionArray = new SgSubscriptExpression (
            RoseHelper::getFileInfo (), addExpression1, buildNullExpression(),
            buildIntVal (1));
        subscriptExpressionReductionArray->set_endOfConstruct (RoseHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (getReductionArrayDeviceName (i)),
            subscriptExpressionReductionArray);

        /*
         * ======================================================
         * The locally computed value to be reduced
         * ======================================================
         */
        
/*        SgSubscriptExpression * subscriptExpressionInputVariable = new SgSubscriptExpression (
            RoseHelper::getFileInfo (), variableDeclarations->getReference (
          getIterationCounterVariableName (1)), variableDeclarations->getReference (
          getIterationCounterVariableName (1)),
            buildIntVal (1));
        subscriptExpressionInputVariable->set_endOfConstruct (RoseHelper::getFileInfo ());
*/
            
         SgExpression * parameterExpression2 = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatLocalName (i)),  variableDeclarations->getReference (
          getIterationCounterVariableName (1)));
        
        
        
        /* Carlo: modified 18/01/12, I don't use anymore warpsize in the reduction subroutine */
        SgExprListExp * actualParameters = buildExprListExp (
            parameterExpression1, parameterExpression2, reductionType );
            
        /*    variableDeclarations->getReference (warpSize), );  */
        
        SgFunctionSymbol * reductionFunctionSymbol =
            isSgFunctionSymbol (
                reductionSubroutines->getHeader (parallelLoop->getReductionTuple (
                    i))->get_symbol_from_symbol_table ());

        ROSE_ASSERT (reductionFunctionSymbol != NULL);

        SgFunctionCallExp * reductionFunctionCall = buildFunctionCallExp (
            reductionFunctionSymbol, actualParameters);

        /*
         * ======================================================
         * Encapsulate the reduction function call in a loop over
         * the dimension of the op_gbl
         * ======================================================         
         */
            
         SgBasicBlock * loopBody = buildBasicBlock (buildExprStatement (reductionFunctionCall));

         SgAssignOp * loopInitialiserExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), buildIntVal (0));

         SgSubtractOp * upperBoundExpression2 = buildSubtractOp (
          buildIntVal( parallelLoop->getOpDatDimension(i) ),
          buildIntVal (1));

      SgFortranDo * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopInitialiserExpression, upperBoundExpression2,
              buildIntVal (1), loopBody);
        
        appendStatement (loopStatement,
            subroutineScope);
        
      }
      else
      {

        /*
         * ======================================================
         * Index into the reduction array on the device
         * ======================================================
         */

        SgSubtractOp * subtractExpression = buildSubtractOp (CUDA::getBlockId (
            BLOCK_X, subroutineScope), buildIntVal (1));

        SgAddOp * sumExpression = buildAddOp (subtractExpression, buildIntVal (1));
            
        SgSubscriptExpression * subscriptExpression = new SgSubscriptExpression (
            RoseHelper::getFileInfo (), sumExpression, buildNullExpression (),
            buildIntVal (1));
        subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (getReductionArrayDeviceName (i)),
            subscriptExpression);

        /*
         * ======================================================
         * The locally computed value to be reduced
         * ======================================================
         */

        SgExpression * parameterExpression2 = variableDeclarations->getReference (
            getOpDatLocalName (i));

        
        /* Carlo: modified 18/01/12, I don't use anymore warpsize in the reduction subroutine */
        SgExprListExp * actualParameters = buildExprListExp (
            parameterExpression1, parameterExpression2, reductionType );
            
        /*    variableDeclarations->getReference (warpSize), reductionType);  */
        
        SgFunctionSymbol * reductionFunctionSymbol =
            isSgFunctionSymbol (
                reductionSubroutines->getHeader (parallelLoop->getReductionTuple (
                    i))->get_symbol_from_symbol_table ());

        ROSE_ASSERT (reductionFunctionSymbol != NULL);

        SgFunctionCallExp * reductionFunctionCall = buildFunctionCallExp (
            reductionFunctionSymbol, actualParameters);

        appendStatement (buildExprStatement (reductionFunctionCall),
            subroutineScope);
      }
    }
  }
}

void
FortranCUDAKernelSubroutine::createCUDAStageInVariablesVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local thread variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = getOpDatLocalName (i);

    if (parallelLoop->isStageInNeeded (i))
    {
      if (parallelLoop->isGlobal (i) && parallelLoop->isArray (i) == false)
      {
        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, parallelLoop->getOpDatBaseType (i),
                subroutineScope));
      }
      else
      {
        if (parallelLoop->isIncrement (i) == true)
        {
          variableDeclarations->add (variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, FortranTypesBuilder::getArray_RankOne (
                      parallelLoop->getOpDatBaseType (i), 0, buildIntVal (
                          parallelLoop->getOpDatDimension (i) - 1)),
                  subroutineScope));
        }
      }
    }
  }
}

void
FortranCUDAKernelSubroutine::createCUDASharedVariableDeclarations ()
{
  using namespace OP2VariableNames;
  using std::find;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA shared variable declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  vector <string> autosharedNames;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i) || (parallelLoop->isDirect (i)
          && parallelLoop->getOpDatDimension (i) > 1))
      {
        string const autosharedVariableName = getSharedMemoryDeclarationName (
            parallelLoop->getOpDatBaseType (i),
            parallelLoop->getSizeOfOpDat (i));

        if (find (autosharedNames.begin (), autosharedNames.end (),
            autosharedVariableName) == autosharedNames.end ())
        {
          Debug::getInstance ()->debugMessage (
              "Creating declaration with name '" + autosharedVariableName
                  + "' for OP_DAT '" + parallelLoop->getOpDatVariableName (i)
                  + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          SgExpression * upperBound = new SgAsteriskShapeExp (
              RoseHelper::getFileInfo ());

          variableDeclarations->add (
              autosharedVariableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  autosharedVariableName,
                  FortranTypesBuilder::getArray_RankOne (
                      parallelLoop->getOpDatBaseType (i), 0, upperBound),
                  subroutineScope, 1, CUDA_SHARED));

          autosharedNames.push_back (autosharedVariableName);

          string const autosharedOffsetVariableName =
              getSharedMemoryOffsetDeclarationName (
                  parallelLoop->getOpDatBaseType (i),
                  parallelLoop->getSizeOfOpDat (i));

          Debug::getInstance ()->debugMessage (
              "Creating offset declaration with name '"
                  + autosharedOffsetVariableName + "' for OP_DAT '"
                  + parallelLoop->getOpDatVariableName (i) + "'",
              Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          variableDeclarations->add (
              autosharedOffsetVariableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  autosharedOffsetVariableName,
                  FortranTypesBuilder::getFourByteInteger (), subroutineScope));
        }
      }
    }
  }
}

FortranCUDAKernelSubroutine::FortranCUDAKernelSubroutine (
    SgScopeStatement * moduleScope, FortranCUDAUserSubroutine * userSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranReductionSubroutines * reductionSubroutines,
    FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranKernelSubroutine (moduleScope, userSubroutine, parallelLoop),
      cardinalitiesDeclaration (cardinalitiesDeclaration),
      dimensionsDeclaration (dimensionsDeclaration), moduleDeclarations (
          moduleDeclarations)
{
  this->reductionSubroutines = reductionSubroutines;

  subroutineHeaderStatement->get_functionModifier ().setCudaKernel ();
}
