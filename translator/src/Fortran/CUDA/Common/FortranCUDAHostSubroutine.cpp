


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


#include "FortranCUDAHostSubroutine.h"
#include "FortranKernelSubroutine.h"
#include "FortranParallelLoop.h"
#include "FortranCUDAOpDatCardinalitiesDeclaration.h"
#include "FortranCUDAModuleDeclarations.h"
#include "FortranOpDatDimensionsDeclaration.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include "OP2.h"
#include "CUDA.h"
#include "PlanFunctionNames.h"

void
FortranCUDAHostSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;

  
  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {

      SgExprStatement * initialiseReductionArraysSize = NULL;
      
      /*
       * ======================================================
       * Direct loops: blocksPerGrid * op_dat dimension
       * Indirect loops: max(plan%nblocks(i)) * op_dat dimension
       * ======================================================
       */
      
      if ( parallelLoop->isDirectLoop () )
      {
        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          variableDeclarations->getReference (CUDA::blocksPerGrid),
          buildIntVal (parallelLoop->getOpDatDimension (i)));

        initialiseReductionArraysSize = buildAssignStatement (
          variableDeclarations->getReference (getReductionCardinalityName (i)),
          multiplyExpression1);
      }
      else
      {
        
        SgPntrArrRefExp * ncolBlkFirst= buildPntrArrRefExp (
          variableDeclarations->getReference (ncolblk), buildIntVal (1));
        
        SgExprStatement * initMaxBlocksPerGrid = buildAssignStatement (variableDeclarations->getReference (maxBlocksPerGrid),
          ncolBlkFirst);
        
        appendStatement (initMaxBlocksPerGrid, subroutineScope);

       /*
        * ======================================================
        * Get the maximum number of blocks per grid with a loop
        * ======================================================
        */

       SgPntrArrRefExp * accessToNColBlk1 = buildPntrArrRefExp (
        variableDeclarations->getReference (ncolblk),
        variableDeclarations->getReference (getIterationCounterVariableName (10)));

       SgPntrArrRefExp * accessToNColBlk2 = buildPntrArrRefExp (
        variableDeclarations->getReference (ncolblk),
        variableDeclarations->getReference (getIterationCounterVariableName (10)));

       SgBasicBlock * ifBody = buildBasicBlock ();
       
       SgExprStatement * assignNewMax = buildAssignStatement (variableDeclarations->getReference (maxBlocksPerGrid),
        accessToNColBlk2);
       
       appendStatement (assignNewMax, ifBody);
        
       SgExpression * ifGuardExpression = buildGreaterOrEqualOp (
          accessToNColBlk1, variableDeclarations->getReference (maxBlocksPerGrid));
       
       SgIfStmt * ifStatement =
        RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

        SgBasicBlock * loopBody = buildBasicBlock (ifStatement);

        SgAssignOp * loopInitialiserExpression = buildAssignOp (
            variableDeclarations->getReference (getIterationCounterVariableName (
                10)), buildIntVal (2));

        SgExpression * upperBoundExpression2 = buildDotExp (variableDeclarations->getReference (
          getActualPlanVariableName (parallelLoop->getUserSubroutineName ())),
            buildOpaqueVarRefExp (ncolors, subroutineScope));

        SgFortranDo * loopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                loopInitialiserExpression, upperBoundExpression2,
                buildIntVal (1), loopBody);

        appendStatement (loopStatement, subroutineScope);                
                
        initialiseReductionArraysSize = buildAssignStatement (
          variableDeclarations->getReference (getReductionCardinalityName (i)),
          buildMultiplyOp (variableDeclarations->getReference (maxBlocksPerGrid),
            buildIntVal (parallelLoop->getOpDatDimension (i))));                    
      }
      
      ROSE_ASSERT ( initialiseReductionArraysSize != NULL );
  
      appendStatement (initialiseReductionArraysSize, subroutineScope);

      FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
          variableDeclarations->getReference (getReductionArrayHostName (i)),
          variableDeclarations->getReference (getReductionCardinalityName (i)),
          subroutineScope);

      FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
          variableDeclarations->getReference (getReductionArrayDeviceName (i)),
          variableDeclarations->getReference (getReductionCardinalityName (i)),
          subroutineScope);

      SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (getReductionArrayHostName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              10)));

      SgExprStatement * assignmentStatement2;

      if (isSgTypeInt (parallelLoop->getOpDatBaseType (i)))
      {
        assignmentStatement2 = buildAssignStatement (arrayIndexExpression1,
            buildIntVal (1));
      }
      else
      {
        assignmentStatement2 = buildAssignStatement (arrayIndexExpression1,
            buildFloatVal (0));
      }
  
       /*
        * ======================================================
        * For OP_INC: reduction array initialised to 0
        * For OP_MAX and OP_MIN: reduction array initialised
        * with global variable value
        * ======================================================
        */

      if ( parallelLoop->isMaximised (i) || parallelLoop->isMinimised (i) )
      {

        SgBasicBlock * innerLoopBody = buildBasicBlock ();
        
        SgAddOp * reductionArrayHostIndex = buildAddOp (
          buildMultiplyOp (
            variableDeclarations->getReference (getIterationCounterVariableName (10)),
            buildIntVal (parallelLoop->getOpDatDimension (i))),
          variableDeclarations->getReference (getIterationCounterVariableName (
                20)));
        
        SgPntrArrRefExp * accessReductionArrayHost = buildPntrArrRefExp (
          variableDeclarations->getReference (getReductionArrayHostName (i)),
          reductionArrayHostIndex);

        SgPntrArrRefExp * accessOpDatHost = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatHostName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (20)));

        SgExprStatement * assignHostValueToDeviceReductionArray =
          buildAssignStatement (accessReductionArrayHost, accessOpDatHost);
          
        appendStatement (assignHostValueToDeviceReductionArray, innerLoopBody);
          
        SgAssignOp * innerLoopInitialiserExpression = buildAssignOp (
            variableDeclarations->getReference (getIterationCounterVariableName (
                20)), buildIntVal (1));
                
        SgExpression * innerUpperBoundExpression = buildIntVal (parallelLoop->getOpDatDimension (i));
                
        SgFortranDo * innerLoopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                innerLoopInitialiserExpression, innerUpperBoundExpression,
                buildIntVal (1), innerLoopBody);
        
        SgBasicBlock * outerLoopBody = buildBasicBlock (innerLoopStatement);

        SgAssignOp * outerLoopInitialiserExpression = buildAssignOp (
            variableDeclarations->getReference (getIterationCounterVariableName (
                10)), buildIntVal (0));

        SgExpression * outerUpperBoundExpression = NULL;
        
        if ( parallelLoop->isDirectLoop () )
        {
          outerUpperBoundExpression = buildSubtractOp (
            variableDeclarations->getReference (CUDA::blocksPerGrid), buildIntVal (1));          
        }
        else
        {
          outerUpperBoundExpression = buildSubtractOp (
            variableDeclarations->getReference (maxBlocksPerGrid), buildIntVal (1));
        }
        SgFortranDo * outerLoopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                outerLoopInitialiserExpression, outerUpperBoundExpression,
                buildIntVal (1), outerLoopBody);
        
        appendStatement (outerLoopStatement, subroutineScope);
                
      }
      else
      {
        
        SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement2);

        SgAssignOp * loopInitialiserExpression = buildAssignOp (
            variableDeclarations->getReference (getIterationCounterVariableName (
                10)), buildIntVal (1));

        SgExpression * upperBoundExpression2 = variableDeclarations->getReference (getReductionCardinalityName (i));

        SgFortranDo * loopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                loopInitialiserExpression, upperBoundExpression2,
                buildIntVal (1), loopBody);

        appendStatement (loopStatement, subroutineScope);

        SgExprStatement * assignmentStatement3 = buildAssignStatement (
            variableDeclarations->getReference (getReductionArrayDeviceName (i)),
            variableDeclarations->getReference (getReductionArrayHostName (i)));

        appendStatement (assignmentStatement3, subroutineScope);
      }
    }
  }
}

void
FortranCUDAHostSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      if (parallelLoop->getOpDatDimension (i) == 1)
      {
        Debug::getInstance ()->debugMessage (
            "Creating statements for OP_DAT argument '"
                + lexical_cast <string> (i) + "'", Debug::FUNCTION_LEVEL,
            __FILE__, __LINE__);

        /*
         * ======================================================
         * Assign device reduction array to host reduction array
         * ======================================================
         */

        SgExprStatement * assignmentStatement1 =
            buildAssignStatement (variableDeclarations->getReference (
                getReductionArrayHostName (i)),
                variableDeclarations->getReference (
                    getReductionArrayDeviceName (i)));

        appendStatement (assignmentStatement1, subroutineScope);

        /*
         * ======================================================
         * Iterate over all elements in the reduction host array
         * and compute either the sum, maximum, or minimum
         * ======================================================
         */

        SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (getReductionArrayHostName (i)),
            variableDeclarations->getReference (
                getIterationCounterVariableName (10)));

        SgExpression * reductionComputationExpression;

        if (parallelLoop->isIncremented (i))
        {
          reductionComputationExpression = buildAddOp (arrayIndexExpression1,
              variableDeclarations->getReference (getOpDatHostName (i)));
        }
        else if (parallelLoop->isMaximised (i))
        {
          SgFunctionSymbol * maxFunctionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("max",
                  subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              arrayIndexExpression1, variableDeclarations->getReference (
                  getOpDatHostName (i)));

          reductionComputationExpression = buildFunctionCallExp (
              maxFunctionSymbol, actualParameters);
        }
        else if (parallelLoop->isMinimised (i))
        {
          SgFunctionSymbol * maxFunctionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("min",
                  subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              arrayIndexExpression1, variableDeclarations->getReference (
                  getOpDatHostName (i)));

          reductionComputationExpression = buildFunctionCallExp (
              maxFunctionSymbol, actualParameters);
        }

        ROSE_ASSERT (reductionComputationExpression != NULL);

        SgExprStatement * assignmentStatement3 = buildAssignStatement (
            variableDeclarations->getReference (getOpDatHostName (i)),
            reductionComputationExpression);

        SgBasicBlock * loopBody = buildBasicBlock ();

        appendStatement (assignmentStatement3, loopBody);

        SgAssignOp * loopInitialiserExpression = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (10)), buildIntVal (0));

        SgSubtractOp * upperBoundExpression =
            buildSubtractOp (variableDeclarations->getReference (
                getReductionCardinalityName (i)), buildIntVal (1));

        SgFortranDo * loopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                loopInitialiserExpression, upperBoundExpression,
                buildIntVal (1), loopBody);

        appendStatement (loopStatement, subroutineScope);

      }
      else
      {
        /*
         * ======================================================
         * When dimension > 1 I need a double nested loop over
         * dimension and blocks
         * ======================================================
         */
        Debug::getInstance ()->debugMessage (
            "Creating statements for OP_DAT argument '"
                + lexical_cast <string> (i) + "'", Debug::FUNCTION_LEVEL,
            __FILE__, __LINE__);

        /*
         * ======================================================
         * Assign device reduction array to host reduction array
         * ======================================================
         */

        SgExprStatement * assignmentStatement1 =
            buildAssignStatement (           
            variableDeclarations->getReference (
                getReductionArrayHostName (i)),
                variableDeclarations->getReference (
                    getReductionArrayDeviceName (i)));

        appendStatement (assignmentStatement1, subroutineScope);

        /*
         * ======================================================
         * Iterate over all elements in the reduction host array
         * and compute either the sum, maximum, or minimum
         * ======================================================
         */

        Debug::getInstance ()->debugMessage (
            "HERE?'"
                + lexical_cast <string> (i) + "'", Debug::FUNCTION_LEVEL,
            __FILE__, __LINE__);

        
        SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatHostName (i)),
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        Debug::getInstance ()->debugMessage (
            "HERE?'"
                + lexical_cast <string> (i) + "'", Debug::FUNCTION_LEVEL,
            __FILE__, __LINE__);
                
                
        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
          buildSubtractOp ( variableDeclarations->getReference (getIterationCounterVariableName (2)), buildIntVal(1)),
          buildIntVal (10));
                
        SgAddOp * indexReductionArrayOnHost = buildAddOp (
          multiplyExpression2, variableDeclarations->getReference (getIterationCounterVariableName (1)));

        SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
          variableDeclarations->getReference (getReductionArrayHostName (i)),
          indexReductionArrayOnHost);

        SgExpression * reductionComputationExpression = NULL;
          
        if (parallelLoop->isIncremented (i))
        {          
          reductionComputationExpression = buildAddOp (
            arrayIndexExpression1, arrayIndexExpression2);
        }
        else if (parallelLoop->isMaximised (i))
        {
          SgFunctionSymbol * maxFunctionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("max",
                  subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              arrayIndexExpression1, arrayIndexExpression2);

          reductionComputationExpression = buildFunctionCallExp (
              maxFunctionSymbol, actualParameters);
        }
        else if (parallelLoop->isMinimised (i))
        {
          SgFunctionSymbol * minFunctionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("min",
                  subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              arrayIndexExpression1, arrayIndexExpression2);

          reductionComputationExpression = buildFunctionCallExp (
              minFunctionSymbol, actualParameters);
        }

        ROSE_ASSERT (reductionComputationExpression != NULL);

        SgExprStatement * assignmentStatement3 = buildAssignStatement (
          buildPntrArrRefExp (variableDeclarations->getReference (getOpDatHostName (i)),
            variableDeclarations->getReference (getIterationCounterVariableName (1))),
            reductionComputationExpression);
            
        SgBasicBlock * loopOverBlocksBody = buildBasicBlock ();

        appendStatement (assignmentStatement3, loopOverBlocksBody);

        SgAssignOp * loopOverBlocksInitialiserExpression = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), buildIntVal (1));

        SgExpression * loopOverBlocksUpperBoundExpression = NULL;

        /*
         * ======================================================
         * For direct loops, the number of blocks is contained
         * in blocksPerGrid
         * For indirect loops, in plan%nblocks
         * ======================================================
         */
        
        if ( parallelLoop->isDirectLoop () )
        {                
          loopOverBlocksUpperBoundExpression = variableDeclarations->getReference (CUDA::blocksPerGrid);
        }
        else
        {
          loopOverBlocksUpperBoundExpression = buildDotExp (variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nblocks, subroutineScope));
        }
        
        SgFortranDo * loopOverBlocksStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                loopOverBlocksInitialiserExpression, loopOverBlocksUpperBoundExpression,
                buildIntVal (1), loopOverBlocksBody);
                
        SgBasicBlock * loopOverDimensionBody = buildBasicBlock ();

        appendStatement (loopOverBlocksStatement, loopOverDimensionBody);

        SgAssignOp * loopOverDimensionInitialiserExpression = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (1));

        SgExpression * loopOverDimensionUpperBoundExpression = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (dim, subroutineScope));
        
        SgFortranDo * loopOverDimensionStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                loopOverDimensionInitialiserExpression, loopOverDimensionUpperBoundExpression,
                buildIntVal (1), loopOverDimensionBody);
                
        appendStatement (loopOverDimensionStatement, subroutineScope);
      }
      
      FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
          variableDeclarations->getReference (getReductionArrayHostName (i)),
          subroutineScope);

      FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
          variableDeclarations->getReference (getReductionArrayDeviceName (i)),
          subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutine::createReductionDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (getIterationCounterVariableName (10),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (10),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (getIterationCounterVariableName (20),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (20),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  /*
   * ======================================================
   * In indirect loops there might be different blocks
   * per grid for each colour: for reduction, we only
   * need the maximum of these numbers
   * ======================================================
   */

  if ( parallelLoop->isDirectLoop () == false )
  {
      variableDeclarations->add (maxBlocksPerGrid,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              maxBlocksPerGrid,
              FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }
          
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      string const reductionArrayHostName = getReductionArrayHostName (i);

      Debug::getInstance ()->debugMessage ("Creating host reduction array '"
          + reductionArrayHostName + "'", Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgVariableDeclaration * reductionArrayHost =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionArrayHostName, FortranTypesBuilder::getArray_RankOne (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope, 1,
              ALLOCATABLE);

      variableDeclarations->add (reductionArrayHostName, reductionArrayHost);

      string const reductionArrayDeviceName = getReductionArrayDeviceName (i);

      Debug::getInstance ()->debugMessage ("Creating device reduction array '"
          + reductionArrayDeviceName + "'", Debug::HIGHEST_DEBUG_LEVEL,
          __FILE__, __LINE__);

      SgVariableDeclaration * reductionArrayDevice =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionArrayDeviceName, FortranTypesBuilder::getArray_RankOne (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope, 2,
              ALLOCATABLE, CUDA_DEVICE);

      variableDeclarations->add (reductionArrayDeviceName, reductionArrayDevice);

      if (parallelLoop->isArray (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating device array for array REDUCTION "
                + lexical_cast <string> (i), Debug::FUNCTION_LEVEL, __FILE__,
            __LINE__);

        string const & variableName = getOpDatHostName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildPointerType (
                    FortranTypesBuilder::getArray_RankOne (
                        parallelLoop->getOpDatBaseType (i))), subroutineScope));
      }
      else
      {
        Debug::getInstance ()->debugMessage (
            "Creating host scalar pointer for scalar REDUCTION "
                + lexical_cast <string> (i), Debug::FUNCTION_LEVEL, __FILE__,
            __LINE__);

        string const & variableName = getOpDatHostName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope));
      }

      string const & reductionCardinalityName = getReductionCardinalityName (i);

      variableDeclarations->add (reductionCardinalityName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionCardinalityName,
              FortranTypesBuilder::getFourByteInteger (), subroutineScope));
    }
  }
}

SgExpression *
FortranCUDAHostSubroutine::getOpDatCardinalityInitialisationExpression (
    SgScopeStatement * scope, unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace OP2::RunTimeVariableNames;
  using namespace OP2VariableNames;

  if (parallelLoop->isDirect (OP_DAT_ArgumentGroup)
      || parallelLoop->isIndirect (OP_DAT_ArgumentGroup))
  {
    SgDotExp * dotExpression1 =
        buildDotExp (variableDeclarations->getReference (getOpDatName (
            OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (dim, scope));

    SgDotExp * dotExpression2 =
        buildDotExp (variableDeclarations->getReference (getOpDatName (
            OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (set, scope));

    SgDotExp * dotExpression3 = buildDotExp (dotExpression2,
        buildOpaqueVarRefExp (size, scope));

    SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
        dotExpression3);

    return multiplyExpression;
  }
  else if (parallelLoop->isReductionRequired (OP_DAT_ArgumentGroup))
  {
    if (parallelLoop->isArray (OP_DAT_ArgumentGroup))
    {
      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (
              OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (dim, scope));

      SgDotExp * dotExpression2 = buildDotExp (
          variableDeclarations->getReference (getOpSetName ()),
          buildOpaqueVarRefExp (size, scope));

      SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
          dotExpression2);

      return multiplyExpression;
    }
    else
    {
      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (getOpSetName ()),
          buildOpaqueVarRefExp (size, scope));

      return dotExpression;
    }
  }
  else if (parallelLoop->isArray (OP_DAT_ArgumentGroup)
      && parallelLoop->isRead (OP_DAT_ArgumentGroup))
  {
    SgDotExp * dotExpression =
        buildDotExp (variableDeclarations->getReference (getOpDatName (
            OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (dim, scope));

    return dotExpression;
  }
}

SgBasicBlock *
FortranCUDAHostSubroutine::createDeallocateStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to deallocate global arrays which are read",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isGlobal (i) && parallelLoop->isArray (i)
          && parallelLoop->isRead (i))
      {
        FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
            variableDeclarations->getReference (getOpDatDeviceName (i)), block);
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAHostSubroutine::createTransferOpDatStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;
  using std::string;
  
  Debug::getInstance ()->debugMessage (
      "Creating statements to transfer OP_DATs onto device",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT dimensions",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDirect (i) || parallelLoop->isIndirect (i))
    {
      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (opDatDimensions),
          dimensionsDeclaration->getOpDatDimensionField (i));

      SgDotExp * dotExpression2 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (dim, block));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          dotExpression1, dotExpression2);

      appendStatement (assignmentStatement, block);
    }
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT cardinalities (on device)",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      FortranParallelLoop * fortranParallelLoop =
          static_cast <FortranParallelLoop *> (parallelLoop);

      if (fortranParallelLoop->isCardinalityDeclarationNeeded (i))
      {
        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (opDatCardinalities),
            cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
                getOpDatCardinalityName (i)));

        SgExpression * rhsOfAssigment =
            getOpDatCardinalityInitialisationExpression (block, i);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression, rhsOfAssigment);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT cardinalities (on host)",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      FortranParallelLoop * fortranParallelLoop =
          static_cast <FortranParallelLoop *> (parallelLoop);

      if (fortranParallelLoop->isCardinalityDeclarationNeeded (i))
      {
        SgExpression * rhsOfAssigment =
            getOpDatCardinalityInitialisationExpression (block, i);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (getOpDatCardinalityName (i)),
            rhsOfAssigment);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to convert OP_DATs between C and Fortran pointers",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * First creates the postfix name
   * ======================================================
   */   
//   boost::crc_32_type result;
//   
//   string uniqueKernelName = "";
//   for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
//   {
//     uniqueKernelName.append ("_");
//     uniqueKernelName.append (parallelLoop->getOpDatVariableName (i));
//   }
//   
//   result.process_bytes (uniqueKernelName.c_str (), uniqueKernelName.length ());
//   
//   string const postfixName = "_" + lexical_cast <string> (result.checksum ());

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);

      
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i) || parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * Statement to convert OP_DAT between C and Fortran
         * pointers
         * ======================================================
         */

        Debug::getInstance ()->debugMessage (
            "Direct/Indirect/Global non-scalar conversion",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgDotExp * parameterExpression1A = buildDotExp (
            variableDeclarations->getReference (getOpDatName (i)),
            buildOpaqueVarRefExp (data_d, block));

        SgVarRefExp * parameterExpression2A = NULL;

/*        if (parallelLoop->isDirectLoop ())
          parameterExpression2A =
              variableDeclarations->getReference (getOpDatDeviceName (i));
        else */


          parameterExpression2A =
             moduleDeclarations->getDeclarations()->getReference (getOpDatDeviceName (i) + 
                parallelLoop->getUserSubroutineName () + postfixName);

        Debug::getInstance ()->debugMessage (
            "Name looked for is: " + getOpDatDeviceName (i) + 
                parallelLoop->getUserSubroutineName () + postfixName,
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);


        SgAggregateInitializer
            * parameterExpression3A =
                FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                    variableDeclarations->getReference (
                        getOpDatCardinalityName (i)));

        SgStatement
            * callStatementA =
                FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                    subroutineScope, parameterExpression1A,
                    parameterExpression2A, parameterExpression3A);

        Debug::getInstance ()->debugMessage (
            "After appending a c_f pointer to: " + getOpDatDeviceName (i) + 
                parallelLoop->getUserSubroutineName () + postfixName,
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

                    
        appendStatement (callStatementA, block);

      }
      else if (parallelLoop->isReductionRequired (i))
      {
        if (parallelLoop->isArray (i))
        {
          Debug::getInstance ()->debugMessage ("Global array conversion",
              Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

          SgDotExp * parameterExpression1A = buildDotExp (
              variableDeclarations->getReference (getOpDatName (i)),
              buildOpaqueVarRefExp (data, block));

          SgVarRefExp * parameterExpression1B =
              variableDeclarations->getReference (getOpDatHostName (i));

          SgDotExp * dotExpression = buildDotExp (
              variableDeclarations->getReference (getOpDatName (i)),
              buildOpaqueVarRefExp (dim, block));

          SgAggregateInitializer * parameterExpression1C =
              FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                  dotExpression);

          SgStatement
              * callStatementA =
                  FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                      subroutineScope, parameterExpression1A,
                      parameterExpression1B, parameterExpression1C);

          appendStatement (callStatementA, block);
        }
        else
        {
          Debug::getInstance ()->debugMessage ("Global scalar conversion",
              Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

          SgDotExp * parameterExpression1A = buildDotExp (
              variableDeclarations->getReference (getOpDatName (i)),
              buildOpaqueVarRefExp (data, block));

          SgVarRefExp * parameterExpression1B =
              variableDeclarations->getReference (getOpDatHostName (i));

          SgStatement
              * callStatementA =
                  FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                      subroutineScope, parameterExpression1A,
                      parameterExpression1B);

          appendStatement (callStatementA, block);
        }
      }
      else
      {
        if (parallelLoop->isRead (i))
        {
          if (parallelLoop->isArray (i))
          {
            Debug::getInstance ()->debugMessage ("Global array conversion",
                Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

            SgDotExp * parameterExpression1A = buildDotExp (
                variableDeclarations->getReference (getOpDatName (i)),
                buildOpaqueVarRefExp (data, block));

            SgVarRefExp * parameterExpression1B =
                variableDeclarations->getReference (getOpDatHostName (i));

            SgAggregateInitializer * parameterExpression1C =
                FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                    variableDeclarations->getReference (
                        getOpDatCardinalityName (i)));

            SgStatement
                * callStatementA =
                    FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                        subroutineScope, parameterExpression1A,
                        parameterExpression1B, parameterExpression1C);

            appendStatement (callStatementA, block);

            SgVarRefExp * arrayExpression = variableDeclarations->getReference (
                getOpDatDeviceName (i));


            SgVarRefExp * upperBound = variableDeclarations->getReference (
                getOpDatCardinalityName (i));

            FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
                arrayExpression, buildIntVal (1), upperBound, block);

            SgExprStatement * assignmentStatement1 = buildAssignStatement (
                variableDeclarations->getReference (getOpDatDeviceName (i)),
                variableDeclarations->getReference (getOpDatHostName (i)));

            appendStatement (assignmentStatement1, block);
          }
          else
          {
            Debug::getInstance ()->debugMessage ("Global scalar conversion",
                Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

            SgDotExp * parameterExpression1A = buildDotExp (
                variableDeclarations->getReference (getOpDatName (i)),
                buildOpaqueVarRefExp (data, block));

            SgVarRefExp * parameterExpression1B =
                variableDeclarations->getReference (getOpDatHostName (i));

            SgStatement
                * callStatementA =
                    FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                        subroutineScope, parameterExpression1A,
                        parameterExpression1B);

            appendStatement (callStatementA, block);
          }
        }
      }
    }
  }

  return block;
}

void
FortranCUDAHostSubroutine::createDataMarshallingDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations for data marshalling",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      FortranParallelLoop * fortranParallelLoop =
          static_cast <FortranParallelLoop *> (parallelLoop);

      if (fortranParallelLoop->isCardinalityDeclarationNeeded (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating OP_DAT size variable for OP_DAT "
                + lexical_cast <string> (i), Debug::HIGHEST_DEBUG_LEVEL,
            __FILE__, __LINE__);

        string const & variableName = getOpDatCardinalityName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getFourByteInteger (),
                subroutineScope));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i) || parallelLoop->isIndirect (i))
      {
/* Carlo: removed because we don't need subroutine scope op_dat device variables (they are declared in the module declaration section) */
/*        Debug::getInstance ()->debugMessage (
            "Creating device array for OP_DAT " + lexical_cast <string> (i),
            Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

        string const & variableName = getOpDatDeviceName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getArray_RankOne (
                    parallelLoop->getOpDatBaseType (i)), subroutineScope, 2,
                CUDA_DEVICE, ALLOCATABLE));*/
      }
      else
      {
        if (parallelLoop->isRead (i))
        {
          if (parallelLoop->isArray (i))
          {
            Debug::getInstance ()->debugMessage (
                "Creating host and device arrays for OP_GBL read "
                    + lexical_cast <string> (i), Debug::FUNCTION_LEVEL,
                __FILE__, __LINE__);

            string const & variableNameOnDevice = getOpDatDeviceName (i);

            Debug::getInstance ()->debugMessage (
                "Creating device array with name " + variableNameOnDevice,
                Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

            variableDeclarations->add (
                variableNameOnDevice,
                FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                    variableNameOnDevice,
                    FortranTypesBuilder::getArray_RankOne (
                        parallelLoop->getOpDatBaseType (i)), subroutineScope,
                    2, CUDA_DEVICE, ALLOCATABLE));

            string const & variableNameOnHost = getOpDatHostName (i);

            Debug::getInstance ()->debugMessage (
                "Creating host array with name " + variableNameOnHost,
                Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

            variableDeclarations->add (
                variableNameOnHost,
                FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                    variableNameOnHost, buildPointerType (
                        FortranTypesBuilder::getArray_RankOne (
                            parallelLoop->getOpDatBaseType (i))),
                    subroutineScope));
          }
          else
          {
            string const & variableName = getOpDatHostName (i);

            variableDeclarations->add (
                variableName,
                FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                    variableName, buildPointerType (
                        parallelLoop->getOpDatBaseType (i)), subroutineScope));
          }
        }
      }
    }
  }
}

void
FortranCUDAHostSubroutine::createCUDAConfigurationLaunchDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration launch local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (CUDA::blocksPerGrid,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::blocksPerGrid, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (CUDA::threadsPerBlock,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::threadsPerBlock, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (CUDA::sharedMemorySize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::sharedMemorySize, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (CUDA::threadSynchRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::threadSynchRet, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));
}

void
FortranCUDAHostSubroutine::createOpDatCardinalitiesDeclaration ()
{
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT cardinalities declaration ", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (opDatCardinalities,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          opDatCardinalities, cardinalitiesDeclaration->getType (),
          subroutineScope, 1, CUDA_DEVICE));
}

void
FortranCUDAHostSubroutine::createOpDatDimensionsDeclaration ()
{
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT dimensions declaration", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (opDatDimensions,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          opDatDimensions, dimensionsDeclaration->getType (), subroutineScope,
          1, CUDA_DEVICE));
}

void
FortranCUDAHostSubroutine::createIterationVariablesDeclarations ()
{
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using std::vector;
  using std::string;
  
  vector <string> fourByteIntegerVariables;

  fourByteIntegerVariables.push_back (getIterationCounterVariableName (1));
  fourByteIntegerVariables.push_back (getIterationCounterVariableName (2));  
  //fourByteIntegerVariables.push_back (getIterationCounterVariableName (10));
  //fourByteIntegerVariables.push_back (getIterationCounterVariableName (20));  
  
  for (vector <string>::iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }  
}


FortranCUDAHostSubroutine::FortranCUDAHostSubroutine (
    SgScopeStatement * moduleScope, FortranKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranHostSubroutine (moduleScope, kernelSubroutine, parallelLoop),
      cardinalitiesDeclaration (dataSizesDeclaration), dimensionsDeclaration (
          opDatDimensionsDeclaration), moduleDeclarations (moduleDeclarations)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaHost ();
}
