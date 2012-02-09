


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


#include "FortranCUDAKernelSubroutineIndirectLoop.h"
#include "FortranCUDAUserSubroutine.h"
#include "FortranParallelLoop.h"
#include "FortranOpDatDimensionsDeclaration.h"
#include "FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop.h"
#include <FortranCUDAModuleDeclarations.h>
#include "FortranTypesBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "RoseHelper.h"
#include "CUDA.h"
#include "Debug.h"
#include <boost/lexical_cast.hpp>

SgStatement *
FortranCUDAKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * parameterExpression;

    if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isIncremented (i))
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with increment access", Debug::OUTER_LOOP_LEVEL,
            __FILE__, __LINE__);

        parameterExpression = variableDeclarations->getReference (
            getOpDatLocalName (i));
      }
      else if (parallelLoop->isRead (i))
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with read access", Debug::OUTER_LOOP_LEVEL,
            __FILE__, __LINE__);

        string const autosharedVariableName = getSharedMemoryDeclarationName (
            parallelLoop->getOpDatBaseType (i),
            parallelLoop->getSizeOfOpDat (i));

        SgAddOp * addExpression1 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (sharedMemoryOffset));
            
        SgPntrArrRefExp * arrayExpression1 =
            buildPntrArrRefExp (moduleDeclarations->getDeclarations ()->getReference (
                getGlobalToLocalMappingName (i) + "_" + parallelLoop->getUserSubroutineName () + postfixName), 
                buildAddOp (addExpression1, buildIntVal (1)));

        SgDotExp * dotExpression1 = buildDotExp (
            variableDeclarations->getReference (opDatDimensions),
            dimensionsDeclaration->getOpDatDimensionField (i));

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
            dotExpression1);

        SgAddOp * addExpression2 = buildAddOp (
            variableDeclarations->getReference (
                getNumberOfBytesVariableName (i)), multiplyExpression1);

        parameterExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (autosharedVariableName),
            addExpression2);
      }
      else
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with read/write or write access",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        string const autosharedVariableName = getSharedMemoryDeclarationName (
            parallelLoop->getOpDatBaseType (i),
            parallelLoop->getSizeOfOpDat (i));

        SgAddOp * addExpression1 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (sharedMemoryOffset));

        SgPntrArrRefExp * arrayExpression1 =
            buildPntrArrRefExp (moduleDeclarations->getDeclarations ()->getReference (            
                getGlobalToLocalMappingName (i) + "_" + parallelLoop->getUserSubroutineName () + postfixName), 
                buildAddOp (addExpression1, buildIntVal (1)));

        SgDotExp * dotExpression1 = buildDotExp (
            variableDeclarations->getReference (opDatDimensions),
            dimensionsDeclaration->getOpDatDimensionField (i));

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
            dotExpression1);

        SgAddOp * addExpression2 = buildAddOp (
            variableDeclarations->getReference (
                getNumberOfBytesVariableName (i)), multiplyExpression1);

        SgAddOp * addExpression3 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (sharedMemoryOffset));

        SgPntrArrRefExp * arrayExpression2 =
            buildPntrArrRefExp (moduleDeclarations->getDeclarations ()->getReference (
                getGlobalToLocalMappingName (i) + "_" + parallelLoop->getUserSubroutineName () + postfixName), 
                buildAddOp (addExpression3, buildIntVal (1)));

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (arrayExpression2,
            dotExpression1);

        SgAddOp * addExpression4 = buildAddOp (
            variableDeclarations->getReference (
                getNumberOfBytesVariableName (i)), multiplyExpression2);

        SgAddOp * addExpression5 = buildAddOp (addExpression4, dotExpression1);

        SgSubscriptExpression * subscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                addExpression2, addExpression5, buildIntVal (1));

        subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        parameterExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (autosharedVariableName),
            buildExprListExp (subscriptExpression));
      }
    }
    else if (parallelLoop->isDirect (i))
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), variableDeclarations->getReference (sharedMemoryOffset));

      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (opDatDimensions),
          dimensionsDeclaration->getOpDatDimensionField (i));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (addExpression1,
          dotExpression);

      SgExpression * accessExpression = NULL;

      if (parallelLoop->getOpDatDimension (i) > 1)
      {
        SgAddOp * addExpression2 = buildAddOp (multiplyExpression,
            dotExpression);

        accessExpression = new SgSubscriptExpression (
            RoseHelper::getFileInfo (),
            buildAddOp (multiplyExpression, buildIntVal (1)),
            buildAddOp (addExpression2, buildIntVal (1)),
            buildIntVal (1));
        accessExpression->set_endOfConstruct (RoseHelper::getFileInfo ());
      }
      else
      {
        accessExpression = multiplyExpression;
      }

      parameterExpression = buildPntrArrRefExp (
          moduleDeclarations->getDeclarations ()->getReference (
          getOpDatName (i) + deviceString + parallelLoop->getUserSubroutineName () + postfixName),
          buildAddOp (accessExpression, buildIntVal(1)));
    }
    else if (parallelLoop->isReductionRequired (i))
    {
      parameterExpression = variableDeclarations->getReference (
          getOpDatLocalName (i));
    }
    else if (parallelLoop->isGlobal (i))
    {
      Debug::getInstance ()->debugMessage ("Read argument",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->isArray (i) == false)
      {
        Debug::getInstance ()->debugMessage (
            "OP_GBL with read access (Scalar)", Debug::HIGHEST_DEBUG_LEVEL,
            __FILE__, __LINE__);

        parameterExpression = variableDeclarations->getReference (getOpDatName (
            i));
      }
      else
      {
        Debug::getInstance ()->debugMessage ("OP_GBL with read access (Array)",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (opDatCardinalities),
            cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
                getOpDatCardinalityName (i)));

        SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
            buildIntVal (1));

        SgSubscriptExpression * subscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (), buildIntVal (
                0), subtractExpression, buildIntVal (1));

        subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        parameterExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatName (i)),
            subscriptExpression);
      }
    }

    ROSE_ASSERT (parameterExpression != NULL);

    actualParameters->append_expression (parameterExpression);
  }

  return buildFunctionCallStmt (userSubroutine->getSubroutineName (),
      buildVoidType (), actualParameters, subroutineScope);
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createIncrementAndWriteAccessEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace LoopVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating increment and write access epilogue statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * Append a syncthreads call if OP_RW or OP_WRITE is
   * present, but no OP_INC is
   * ======================================================
   */

  
  bool hasIncrementedArg = false;
  bool hasWriteOrReadWrite = false;
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (i) ) hasWriteOrReadWrite = true;
    if (parallelLoop->isIncremented (i)) hasIncrementedArg = true;
  }

  if (hasWriteOrReadWrite && !hasIncrementedArg)
    appendStatement (buildExprStatement (CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      block);
  
  unsigned int pindOffsOffset = 0;

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);  
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (i)
            || parallelLoop->isIncremented (i))
        {
          string const autosharedVariableName = getSharedMemoryDeclarationName (
              parallelLoop->getOpDatBaseType (i), parallelLoop->getSizeOfOpDat (
                  i));
          /*
           * ======================================================
           * Defining lower and upper bounds and increment
           * ======================================================
           */

          SgDotExp * dotExpression2 = buildDotExp (
              variableDeclarations->getReference (opDatDimensions),
              dimensionsDeclaration->getOpDatDimensionField (i));

          SgMultiplyOp * multiplyExpression1 =
              buildMultiplyOp (variableDeclarations->getReference (
                  getIndirectOpDatSizeName (i)), dotExpression2);

          SgLessThanOp * lessThanExpression1 = buildLessThanOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), multiplyExpression1);

          SgSubtractOp * subtractExpression1 = buildSubtractOp (
              CUDA::getThreadId (THREAD_X, subroutineScope), buildIntVal (1));

          SgExprStatement * assignmentStatement1 = buildAssignStatement (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), subtractExpression1);

          appendStatement (assignmentStatement1, block);

          /*
           * ======================================================
           * Defining modulo result assignment
           * ======================================================
           */

          SgFunctionSymbol * functionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("mod",
                  subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), dotExpression2);

          SgFunctionCallExp * functionCall = buildFunctionCallExp (
              functionSymbol, actualParameters);

          SgExprStatement * assignmentStatement2 = buildAssignStatement (
              variableDeclarations->getReference (moduloResult), functionCall);

          /*
           * ======================================================
           * Defining device variable accessing expressions
           * ======================================================
           */

          SgMultiplyOp * multiplyExpression2a = buildMultiplyOp (
              variableDeclarations->getReference (blockID), buildIntVal (
                  parallelLoop->getNumberOfDistinctIndirectOpDats ()));

          SgAddOp * addExpression2a = buildAddOp (buildIntVal (pindOffsOffset),
              multiplyExpression2a);

          SgPntrArrRefExp * arrayIndexExpression2a = buildPntrArrRefExp (
              variableDeclarations->getReference (pindOffs), addExpression2a);

          SgDivideOp * divideExpression2 = buildDivideOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), dotExpression2);

          SgAddOp * addExpression2b = buildAddOp (arrayIndexExpression2a,
              divideExpression2);

          SgAddOp * addExpression2c = buildAddOp (buildIntVal (0),
              addExpression2b);

          SgPntrArrRefExp * arrayIndexExpression2b = buildPntrArrRefExp (
              moduleDeclarations->getDeclarations ()->getReference (
              getLocalToGlobalMappingName (i) + "_" + parallelLoop->getUserSubroutineName () + postfixName),
              buildAddOp (addExpression2c, buildIntVal (1)));

          SgMultiplyOp * multiplyExpression2b = buildMultiplyOp (
              arrayIndexExpression2b, dotExpression2);

          SgAddOp * addExpression2d = buildAddOp (
              variableDeclarations->getReference (moduloResult),
              multiplyExpression2b);

          SgPntrArrRefExp * arrayIndexExpression2c = buildPntrArrRefExp (
              moduleDeclarations->getDeclarations ()->getReference (
              getOpDatName (i) + deviceString + parallelLoop->getUserSubroutineName () + postfixName),
              buildAddOp (addExpression2d, buildIntVal (1)));

          /*
           * ======================================================
           * Defining shared variable accessing expressions
           * ======================================================
           */

          SgAddOp * addExpression2e = buildAddOp (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          SgPntrArrRefExp * arrayIndexExpression2d = buildPntrArrRefExp (
              variableDeclarations->getReference (autosharedVariableName),
              addExpression2e);

          SgExprStatement * assignmentStatement3 = NULL;

          if (parallelLoop->isIncremented (i))
          {
            SgAddOp * addExpression2f = buildAddOp (arrayIndexExpression2c,
                arrayIndexExpression2d);

            assignmentStatement3 = buildAssignStatement (
                arrayIndexExpression2c, addExpression2f);
          }

          if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (i))
          {
            assignmentStatement3 = buildAssignStatement (
                arrayIndexExpression2c, arrayIndexExpression2d);
          }

          /*
           * ======================================================
           * Defining loop body
           * ======================================================
           */

          SgAddOp * addExpression2f = buildAddOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)),
              CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

          SgExprStatement * assignmentStatement4 = buildAssignStatement (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), addExpression2f);

          SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement2,
              assignmentStatement3, assignmentStatement4);

          SgWhileStmt * loopStatement = buildWhileStmt (lessThanExpression1,
              loopBody);

          loopStatement->set_has_end_statement (true);

          appendStatement (loopStatement, block);

        }
        pindOffsOffset++;
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createStageOutFromLocalMemoryToSharedMemoryStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating stage out from local memory to shared memory statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgBasicBlock * ifBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      string const autosharedVariableName = getSharedMemoryDeclarationName (
          parallelLoop->getOpDatBaseType (i), parallelLoop->getSizeOfOpDat (i));

      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (opDatDimensions),
          dimensionsDeclaration->getOpDatDimensionField (i));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          variableDeclarations->getReference (getIncrementAccessMapName (i)),
          dotExpression);

      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), multiplyExpression);

      SgAddOp * addExpression2 =
          buildAddOp (variableDeclarations->getReference (
              getNumberOfBytesVariableName (i)), addExpression1);

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
          variableDeclarations->getReference (autosharedVariableName),
          addExpression2);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
          variableDeclarations->getReference (autosharedVariableName),
          addExpression2);

      SgAddOp * addExpression3 =
          buildAddOp (arrayExpression2, arrayExpression1);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression3, addExpression3);

      appendStatement (assignmentStatement, innerLoopBody);

      SgAssignOp * innerLoopLowerBoundExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgSubtractOp * innerLoopUpperBoundExpression = buildSubtractOp (
          dotExpression, buildIntVal (1));

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopLowerBoundExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      appendStatement (innerLoopStatement, ifBody);
    }
  }

  SgEqualityOp * ifGuardExpression = buildEqualityOp (
      variableDeclarations->getReference (colour2),
      variableDeclarations->getReference (colour1));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (ifStatement, loopBody);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      loopBody);

  SgAssignOp * lowerBoundExpression = buildAssignOp (
      variableDeclarations->getReference (colour1), buildIntVal (0));

  SgSubtractOp * upperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (numberOfColours), buildIntVal (1));

  SgFortranDo
      * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              lowerBoundExpression, upperBoundExpression, buildIntVal (1),
              loopBody);

  appendStatement (loopStatement, block);

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createIncrementAdjustmentStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to adjust incremented OP_DATS",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);  
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      Debug::getInstance ()->debugMessage ("Creating statements for OP_DAT "
          + lexical_cast <string> (i), Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
          getIterationCounterVariableName (1)),
          variableDeclarations->getReference (sharedMemoryOffset));

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          moduleDeclarations->getDeclarations ()->getReference (
          getGlobalToLocalMappingName (i) + "_" + parallelLoop->getUserSubroutineName () + postfixName),
          buildAddOp (addExpression, buildIntVal (1)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          variableDeclarations->getReference (getIncrementAccessMapName (i)),
          arrayExpression);

      appendStatement (assignmentStatement, block);
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseIncrementAccessStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise scratchpad memory",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      Debug::getInstance ()->debugMessage ("Creating statements for OP_DAT "
          + lexical_cast <string> (i), Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgBasicBlock * loopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression, buildIntVal (0));

      appendStatement (assignmentStatement, loopBody);

      SgAssignOp * lowerBoundExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (opDatDimensions),
          dimensionsDeclaration->getOpDatDimensionField (i));

      SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      SgFortranDo * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              lowerBoundExpression, upperBoundExpression, buildIntVal (1),
              loopBody);

      appendStatement (loopStatement, block);
    }
  }

  return block;
}

void
FortranCUDAKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating main execution loop statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  if (parallelLoop->hasIncrementedOpDats ())
  {
    SgExprStatement * assignmentStatement1 = buildAssignStatement (
        variableDeclarations->getReference (colour2), buildIntVal (-1));

    appendStatement (assignmentStatement1, loopBody);

    SgBasicBlock * ifBody = buildBasicBlock ();

    appendStatement (createInitialiseIncrementAccessStatements (), ifBody);

    appendStatement (createUserSubroutineCallStatement (), ifBody);

    SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
        getIterationCounterVariableName (1)),
        variableDeclarations->getReference (sharedMemoryOffset));

    SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
        variableDeclarations->getReference (pthrcol), addExpression1);

    SgExprStatement * assignmentStatement2 = buildAssignStatement (
        variableDeclarations->getReference (colour2), arrayExpression1);

    appendStatement (assignmentStatement2, ifBody);

    SgExpression * ifGuardExpression =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)),
            variableDeclarations->getReference (numberOfActiveThreads));

    SgIfStmt * ifStatement =
        RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
            ifGuardExpression, ifBody);

    appendStatement (ifStatement, loopBody);

    appendStatement (createIncrementAdjustmentStatements (), loopBody);

    appendStatement (createStageOutFromLocalMemoryToSharedMemoryStatements (),
        loopBody);
  }
  else
  {
    appendStatement (createUserSubroutineCallStatement (), loopBody);
  }

  SgAddOp * addExpresssion1 = buildAddOp (variableDeclarations->getReference (
      getIterationCounterVariableName (1)), CUDA::getThreadBlockDimension (
      THREAD_X, subroutineScope));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      addExpresssion1);

  appendStatement (assignmentStatement1, loopBody);

  if (parallelLoop->hasIncrementedOpDats ())
  {
    SgLessThanOp * loopGuardExpression =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)),
            variableDeclarations->getReference (numberOfActiveThreadsCeiling));

    SgWhileStmt * loopStatement =
        buildWhileStmt (loopGuardExpression, loopBody);

    loopStatement->set_has_end_statement (true);

    appendStatement (loopStatement, subroutineScope);
  }
  else
  {
    SgLessThanOp * loopGuardExpression =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)),
            variableDeclarations->getReference (numberOfActiveThreads));

    SgWhileStmt * loopStatement =
        buildWhileStmt (loopGuardExpression, loopBody);

    loopStatement->set_has_end_statement (true);

    appendStatement (loopStatement, subroutineScope);
  }
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseCUDASharedVariablesStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise shared memory", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  unsigned int pindOffsOffset = 0;

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const autosharedVariableName = getSharedMemoryDeclarationName (
            parallelLoop->getOpDatBaseType (i),
            parallelLoop->getSizeOfOpDat (i));

        /*
         * ======================================================
         * Initialise the lower bound of the while loop
         * ======================================================
         */

        Debug::getInstance ()->debugMessage (
            "Initialise the lower bound of the while loop",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgSubtractOp * subtractExpression1 = buildSubtractOp (
            CUDA::getThreadId (THREAD_X, subroutineScope), buildIntVal (1));

        SgExprStatement * assignmentStatement1 = buildAssignStatement (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), subtractExpression1);

        appendStatement (assignmentStatement1, block);

        /*
         * ======================================================
         * Initialise the upper bound of the while loop
         * ======================================================
         */

        Debug::getInstance ()->debugMessage (
            "Initialise the upper bound of the while loop",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgDotExp * dotExpression2 = buildDotExp (
            variableDeclarations->getReference (opDatDimensions),
            dimensionsDeclaration->getOpDatDimensionField (i));

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)),
            dotExpression2);

        SgExprStatement * assignmentStatement2 = buildAssignStatement (
            variableDeclarations->getReference (getUpperBoundVariableName (1)),
            multiplyExpression2);

        appendStatement (assignmentStatement2, block);

        /*
         * ======================================================
         * Statement to calculate modulus
         * ======================================================
         */

        Debug::getInstance ()->debugMessage ("While loop body",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgBasicBlock * loopBody = buildBasicBlock ();

        if (parallelLoop->isIncremented (i) == false)
        {
          SgFunctionSymbol * modFunctionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("mod",
                  subroutineScope);

          SgDotExp * dotExpression3 = buildDotExp (
              variableDeclarations->getReference (opDatDimensions),
              dimensionsDeclaration->getOpDatDimensionField (i));

          SgExprListExp * modActualParameters = buildExprListExp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), dotExpression3);

          SgFunctionCallExp * modFunctionCall = buildFunctionCallExp (
              modFunctionSymbol, modActualParameters);

          SgExprStatement * assignmentStatement3 = buildAssignStatement (
              variableDeclarations->getReference (moduloResult),
              modFunctionCall);

          appendStatement (assignmentStatement3, loopBody);

          /*
           * ======================================================
           * Statement to index shared memory array
           * ======================================================
           */
          Debug::getInstance ()->debugMessage (
              "Statement to index shared memory", Debug::HIGHEST_DEBUG_LEVEL,
              __FILE__, __LINE__);

          SgAddOp * addExpression4a = buildAddOp (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          SgPntrArrRefExp * arrayIndexExpression4a = buildPntrArrRefExp (
              variableDeclarations->getReference (autosharedVariableName),
              addExpression4a);

          SgMultiplyOp * multiplyExpression4a = buildMultiplyOp (
              variableDeclarations->getReference (blockID), buildIntVal (
                  parallelLoop->getNumberOfDistinctIndirectOpDats ()));

          SgAddOp * addExpression4b = buildAddOp (buildIntVal (pindOffsOffset),
              multiplyExpression4a);

          SgPntrArrRefExp * arrayIndexExpression4b = buildPntrArrRefExp (
              variableDeclarations->getReference (pindOffs), addExpression4b);

          SgDotExp * dotExpression4 = buildDotExp (
              variableDeclarations->getReference (opDatDimensions),
              dimensionsDeclaration->getOpDatDimensionField (i));

          SgDivideOp * divideExpression4 = buildDivideOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), dotExpression4);

          SgAddOp * addExpression4c = buildAddOp (arrayIndexExpression4b,
              divideExpression4);

          SgAddOp * addExpression4d = buildAddOp (buildIntVal (0),
              addExpression4c);

          SgPntrArrRefExp * arrayIndexExpression4c = buildPntrArrRefExp (
              moduleDeclarations->getDeclarations ()->getReference (getLocalToGlobalMappingName (
                  i) + "_" + parallelLoop->getUserSubroutineName () + postfixName ),
              buildAddOp (addExpression4d, buildIntVal (1)));

          SgMultiplyOp * multiplyExpression4b = buildMultiplyOp (
              arrayIndexExpression4c, dotExpression4);

          SgAddOp * addExpression4e = buildAddOp (
              variableDeclarations->getReference (moduloResult),
              multiplyExpression4b);

          SgPntrArrRefExp * arrayIndexExpression4d = buildPntrArrRefExp (
              moduleDeclarations->getDeclarations ()->getReference (
              getOpDatName (i) + deviceString + parallelLoop->getUserSubroutineName () + postfixName),
              buildAddOp (addExpression4e, buildIntVal (1)));

          SgExprStatement * assignmentStatement4 = buildAssignStatement (
              arrayIndexExpression4a, arrayIndexExpression4d);

          appendStatement (assignmentStatement4, loopBody);

          pindOffsOffset++;
        }
        else
        {
          /*
           * ======================================================
           * Statement to index shared memory
           * ======================================================
           */
          SgAddOp * addExpression4 = buildAddOp (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (
              variableDeclarations->getReference (autosharedVariableName),
              addExpression4);

          SgExprStatement * assignmentStatement4 = buildAssignStatement (
              arrayIndexExpression4, buildIntVal (0));

          appendStatement (assignmentStatement4, loopBody);
        }

        /*
         * ======================================================
         * Statement to increment loop counter
         * ======================================================
         */
        SgAddOp * addExpression5 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

        SgExprStatement * assignmentStatement5 = buildAssignStatement (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), addExpression5);

        appendStatement (assignmentStatement5, loopBody);

        /*
         * ======================================================
         * While loop guard
         * ======================================================
         */
        SgExpression * loopGuard = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (getUpperBoundVariableName (1)));

        SgWhileStmt * whileStatement = buildWhileStmt (loopGuard, loopBody);

        whileStatement->set_has_end_statement (true);

        appendStatement (whileStatement, block);
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createIncrementAccessThreadZeroStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating thread zero statements for incremented OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (
      variableDeclarations->getReference (numberOfActiveThreads), buildIntVal (
          1));

  SgDivideOp * divideExpression1 = buildDivideOp (subtractExpression1,
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

  SgExpression * addExpression1 = buildAddOp (buildIntVal (1),
      divideExpression1);

  SgMultiplyOp * multiplyExpression1 =
      buildMultiplyOp (
          CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
          addExpression1);

  SgStatement * statement1 = buildAssignStatement (buildOpaqueVarRefExp (
      numberOfActiveThreadsCeiling, subroutineScope), multiplyExpression1);

  appendStatement (statement1, block);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (pnthrcol, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgStatement * statement2 = buildAssignStatement (buildOpaqueVarRefExp (
      numberOfColours, subroutineScope), arrayExpression2);

  appendStatement (statement2, block);

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseBytesPerOpDatStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise bytes per OP_DAT",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * Initialise round-up variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Initialising round-up variable for '"
                + parallelLoop->getOpDatVariableName (i) + "'",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (opDatDimensions),
            dimensionsDeclaration->getOpDatDimensionField (i));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)),
            dotExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (getRoundUpVariableName (i)),
            multiplyExpression);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  /*
   * ======================================================
   * Initialise number of bytes variables
   * ======================================================
   */

  bool firstInitialization = true;

  for (unsigned int i = 1, lasti = 1; i
      <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      Debug::getInstance ()->debugMessage (
          "Initialising number of bytes variable for '"
              + parallelLoop->getOpDatVariableName (i) + "'",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->isIndirect (i))
      {
        if (firstInitialization)
        {
          firstInitialization = false;

          SgExprStatement * assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), buildIntVal (0));

          appendStatement (assignmentStatement, block);
        }
        else
        {
          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  lasti)), buildIntVal (parallelLoop->getSizeOfOpDat (lasti)));

          SgDivideOp * divideExpression1 = buildDivideOp (multiplyExpression1,
              buildIntVal (parallelLoop->getSizeOfOpDat (i)));

          SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
              variableDeclarations->getReference (
                  getRoundUpVariableName (lasti)), buildIntVal (
                  parallelLoop->getSizeOfOpDat (lasti)));

          SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression2,
              buildIntVal (parallelLoop->getSizeOfOpDat (i)));

          SgAddOp * addExpression = buildAddOp (divideExpression1,
              divideExpression2);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), addExpression);

          appendStatement (assignmentStatement, block);
        }

        lasti = i;
      }
    }
  }

  return block;
}

SgIfStmt *
FortranCUDAKernelSubroutineIndirectLoop::createThreadZeroStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage ("Creating thread zero statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * ifBlock = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (CUDA::getBlockId (
      BLOCK_X, subroutineScope), buildIntVal (1));

  SgAddOp * arrayIndexExpression1 = buildAddOp (subtractExpression1,
      variableDeclarations->getReference (blockOffset));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (pblkMap), arrayIndexExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (blockID), arrayExpression1);

  appendStatement (assignmentStatement1, ifBlock);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (pnelems, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (numberOfActiveThreads),
      arrayExpression2);

  appendStatement (assignmentStatement2, ifBlock);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  if (parallelLoop->hasIncrementedOpDats ())
  {
    appendStatement (createIncrementAccessThreadZeroStatements (), ifBlock);
  }

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (poffset, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      buildOpaqueVarRefExp (sharedMemoryOffset, subroutineScope),
      arrayExpression3);

  appendStatement (assignmentStatement3, ifBlock);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (blockID), buildIntVal (
                parallelLoop->getNumberOfDistinctIndirectOpDats ()));

        SgAddOp * addExpression = buildAddOp (buildIntVal (offset),
            multiplyExpression);

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (pindSizes), addExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)),
            arrayExpression);

        appendStatement (assignmentStatement, ifBlock);

        ++offset;
      }
    }
  }

  /*
   * ======================================================
   * Add the if statement with a NULL else block
   * ======================================================
   */

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildSubtractOp (
      CUDA::getThreadId (THREAD_X, subroutineScope), buildIntVal (1)),
      buildIntVal (0));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  return ifStatement;
}

void
FortranCUDAKernelSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;

  appendStatement (createThreadZeroStatements (), subroutineScope);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  appendStatement (createInitialiseBytesPerOpDatStatements (), subroutineScope);

  appendStatement (createInitialiseCUDASharedVariablesStatements (),
      subroutineScope);

  createReductionLocalVariableInitialisation ();
      
  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  SgSubtractOp * subtractExpression1 = buildSubtractOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      subtractExpression1);

  appendStatement (assignmentStatement1, subroutineScope);

  createExecutionLoopStatements ();

  appendStatement (createIncrementAndWriteAccessEpilogueStatements (),
      subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
{
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for incremented OP_DATS",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      Debug::getInstance ()->debugMessage (
          "Creating increment access mapping for OP_DAT " + lexical_cast <
              string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      string const variableName = getIncrementAccessMapName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  variableDeclarations->add (numberOfColours,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfColours, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, 1, CUDA_SHARED));

  variableDeclarations->add (numberOfActiveThreadsCeiling,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfActiveThreadsCeiling,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope, 1,
          CUDA_SHARED));

  variableDeclarations ->add (colour1,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          colour1, FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (colour2,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          colour2, FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (getIterationCounterVariableName (2),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranCUDAKernelSubroutineIndirectLoop::createExecutionLocalVariableDeclarations ()
{
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed to execute kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (sharedMemoryOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          sharedMemoryOffset, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, 1, CUDA_SHARED));

  variableDeclarations->add (blockID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          blockID, FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          1, CUDA_SHARED));

  variableDeclarations->add (numberOfActiveThreads,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfActiveThreads, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, 1, CUDA_SHARED));

  variableDeclarations ->add (moduloResult,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          moduloResult, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations ->add (nbytes,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          nbytes, FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (getIterationCounterVariableName (1),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (getUpperBoundVariableName (1),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getUpperBoundVariableName (1),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranCUDAKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createCUDAStageInVariablesVariableDeclarations ();

  createCUDASharedVariableDeclarations ();

  createExecutionLocalVariableDeclarations ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = getNumberOfBytesVariableName (i);

    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      Debug::getInstance ()->debugMessage (
          "Creating number of bytes declaration for OP_DAT " + lexical_cast <
              string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);

      variableDeclarations->add (variableName, variableDeclaration);

      numberOfBytesDeclarations[parallelLoop->getOpDatVariableName (i)]
          = variableDeclaration;
    }
    else
    {
      Debug::getInstance ()->debugMessage (
          "Number of bytes declaration NOT needed for OP_DAT " + lexical_cast <
              string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      variableDeclarations ->add (variableName,
          numberOfBytesDeclarations[parallelLoop->getOpDatVariableName (i)]);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      Debug::getInstance ()->debugMessage ("Creating round up declaration "
          + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
          __LINE__);

      string const & variableName = getRoundUpVariableName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating size argument for OP_DAT " + lexical_cast <string> (i),
            Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

        string const variableName = getIndirectOpDatSizeName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getFourByteInteger (),
                subroutineScope, 1, CUDA_SHARED));
      }
    }
  }

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementAccessLocalVariableDeclarations ();
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating plan formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Another plan function formal parameter
   * ======================================================
   */

  SgDotExp * dotExpression1 = buildDotExp (variableDeclarations->getReference (
      opDatCardinalities),
      cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
          pindSizesSize));

  SgSubtractOp * upperBoundExpression1 = buildSubtractOp (dotExpression1,
      buildIntVal (1));

  variableDeclarations->add (
      pindSizes,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pindSizes,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression1), subroutineScope, formalParameters, 1,
          CUDA_DEVICE));

  /*
   * ======================================================
   * Another plan function formal parameter
   * ======================================================
   */

  SgDotExp * dotExpression2 = buildDotExp (variableDeclarations->getReference (
      opDatCardinalities),
      cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
          pindOffsSize));

  SgSubtractOp * upperBoundExpression2 = buildSubtractOp (dotExpression2,
      buildIntVal (1));

  variableDeclarations->add (
      pindOffs,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pindOffs,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression2), subroutineScope, formalParameters, 1,
          CUDA_DEVICE));

  /*
   * ======================================================
   * Another plan function formal parameter
   * ======================================================
   */

  SgDotExp * dotExpression3 = buildDotExp (variableDeclarations->getReference (
      opDatCardinalities),
      cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
          pblkMapSize));

  SgSubtractOp * upperBoundExpression3 = buildSubtractOp (dotExpression3,
      buildIntVal (1));

  variableDeclarations->add (
      pblkMap,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pblkMap,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression3), subroutineScope, formalParameters, 1,
          CUDA_DEVICE));

  /*
   * ======================================================
   * Another plan function formal parameter
   * ======================================================
   */

  SgDotExp * dotExpression4 = buildDotExp (variableDeclarations->getReference (
      opDatCardinalities),
      cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
          poffsetSize));

  SgSubtractOp * upperBoundExpression4 = buildSubtractOp (dotExpression4,
      buildIntVal (1));

  variableDeclarations->add (
      poffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          poffset,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression4), subroutineScope, formalParameters, 1,
          CUDA_DEVICE));

  /*
   * ======================================================
   * Another plan function formal parameter
   * ======================================================
   */

  SgDotExp * dotExpression5 = buildDotExp (variableDeclarations->getReference (
      opDatCardinalities),
      cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
          pnelemsSize));

  SgSubtractOp * upperBoundExpression5 = buildSubtractOp (dotExpression5,
      buildIntVal (1));

  variableDeclarations->add (
      pnelems,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pnelems,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression5), subroutineScope, formalParameters, 1,
          CUDA_DEVICE));

  /*
   * ======================================================
   * Another plan function formal parameter
   * ======================================================
   */

  SgDotExp * dotExpression6 = buildDotExp (variableDeclarations->getReference (
      opDatCardinalities),
      cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
          pnthrcolSize));

  SgSubtractOp * upperBoundExpression6 = buildSubtractOp (dotExpression6,
      buildIntVal (1));

  variableDeclarations->add (
      pnthrcol,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pnthrcol,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression6), subroutineScope, formalParameters, 1,
          CUDA_DEVICE));

  /*
   * ======================================================
   * Another plan function formal parameter
   * ======================================================
   */

  SgDotExp * dotExpression7 = buildDotExp (variableDeclarations->getReference (
      opDatCardinalities),
      cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
          pthrcolSize));

  SgSubtractOp * upperBoundExpression7 = buildSubtractOp (dotExpression7,
      buildIntVal (1));

  variableDeclarations->add (
      pthrcol,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pthrcol,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression7), subroutineScope, formalParameters, 1,
          CUDA_DEVICE));

  /*
   * ======================================================
   * Another plan function formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      blockOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          blockOffset, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters, 1, VALUE));
}

void
FortranCUDAKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace SageBuilder;
  using std::string;
  using boost::lexical_cast;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters for " + 
    lexical_cast<string> (parallelLoop->getNumberOfOpDatArgumentGroups ()) + " parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i))
      {
        string const & variableName = getReductionArrayDeviceName (i);

	Debug::getInstance ()->debugMessage ("Reduction for parameter: " + variableName,
          Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

        variableDeclarations->add (
            variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, FortranTypesBuilder::getArray_RankOne (
                    parallelLoop->getOpDatBaseType (i)), subroutineScope,
                formalParameters, 1, CUDA_DEVICE));
      }
      else if (parallelLoop->isIndirect (i) || parallelLoop->isDirect (i))
      {
/* Carlo: deleted these formal parameters, as they are declared in the module declaration section */
        string const variableName = getOpDatName (i);
	Debug::getInstance ()->debugMessage ("Should add opDat, but it is declared in mod. decl. sect.: " + variableName,
          Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
	/*
        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (opDatCardinalities),
            cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
                getOpDatCardinalityName (i)));

        SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
            buildIntVal (1));

        variableDeclarations->add (
            variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName,
                FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                    parallelLoop->getOpDatBaseType (i), buildIntVal (0),
                    upperBoundExpression), subroutineScope, formalParameters,
                1, CUDA_DEVICE));
*/
      }
      else if (parallelLoop->isRead (i))
      {
        string const & variableName = getOpDatName (i);

	Debug::getInstance ()->debugMessage ("Deb formal param: " + variableName,
	  Debug::FUNCTION_LEVEL, __FILE__, __LINE__);	

        if (parallelLoop->isArray (i))
        {
          variableDeclarations->add (
              variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                  variableName,
                  FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                      parallelLoop->getOpDatBaseType (i), buildIntVal (0),
                      buildIntVal (parallelLoop->getOpDatDimension (i) - 1)),
                  subroutineScope, formalParameters, 1, CUDA_DEVICE));
        }
        else
        {
          variableDeclarations->add (
              variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                  variableName, parallelLoop->getOpDatBaseType (i),
                  subroutineScope, formalParameters, 1, VALUE));

        }
      }
    }
  }

/* Carlo: deleted these formal parameters, as they are declared in the module declaration section */
/*  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const & variableName = getLocalToGlobalMappingName (i);

        SgDotExp * dotExpression2 = buildDotExp (
            variableDeclarations->getReference (opDatCardinalities),
            cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
                getLocalToGlobalMappingSizeName (i)));

        SgSubtractOp * upperBoundExpression2 = buildSubtractOp (dotExpression2,
            buildIntVal (1));

        variableDeclarations ->add (
            variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName,
                FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                    FortranTypesBuilder::getFourByteInteger (),
                    buildIntVal (0), upperBoundExpression2), subroutineScope,
                formalParameters, 1, CUDA_DEVICE));
      }
    }
  }
*/
/* Carlo: deleted these formal parameters, as they are declared in the module declaration section */
/*  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      string const & variableName = getGlobalToLocalMappingName (i);

      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (opDatCardinalities),
          cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
              getGlobalToLocalMappingSizeName (i)));

      SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      variableDeclarations->add (
          variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName,
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  FortranTypesBuilder::getTwoByteInteger (),
                  lowerBoundExpression, upperBoundExpression), subroutineScope,
              formalParameters, 1, CUDA_DEVICE));
    }
  }
*/
}

void
FortranCUDAKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (
      opDatDimensions,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opDatDimensions, dimensionsDeclaration->getType (), subroutineScope,
          formalParameters, 1, CUDA_DEVICE));

  variableDeclarations->add (
      opDatCardinalities,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opDatCardinalities, cardinalitiesDeclaration->getType (),
          subroutineScope, formalParameters, 1, CUDA_DEVICE));

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
}

FortranCUDAKernelSubroutineIndirectLoop::FortranCUDAKernelSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    FortranCUDAUserSubroutine * userSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranReductionSubroutines * reductionSubroutines,
    FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop * cardinalitiesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranCUDAKernelSubroutine (moduleScope, userSubroutine, parallelLoop,
      reductionSubroutines, cardinalitiesDeclaration, dimensionsDeclaration,
      moduleDeclarations)
{
  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
