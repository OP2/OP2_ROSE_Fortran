


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


#include "FortranOpenMPKernelSubroutineIndirectLoop.h"
#include "FortranParallelLoop.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "RoseHelper.h"
#include "Debug.h"
#include "OpenMP.h"

SgStatement *
FortranOpenMPKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using boost::lexical_cast;
  using std::string;
  
  Debug::getInstance ()->debugMessage ("Creating call to user kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    Debug::getInstance ()->debugMessage ("Considering parameter" + lexical_cast<string> (i),
        Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    
    if (parallelLoop->isReductionRequired (i))
    {
      actualParameters->append_expression (variableDeclarations->getReference (
          getOpDatName (i)));
    }
    else if (parallelLoop->isGlobal (i))
    {
      actualParameters->append_expression (variableDeclarations->getReference (
          getOpDatName (i)));
    }
    else if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isIncremented (i))
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (getOpDatLocalName (i)));
      }
      else
      {
        SgAddOp * addExpression1 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (OpenMP::threadBlockOffset));

        SgPntrArrRefExp * arrayExpression1 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                getGlobalToLocalMappingName (i)), addExpression1);

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (arrayExpression1,
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression2 = buildAddOp (buildIntVal (1),
            multiplyExpression);
            
        /*
         * ======================================================
         * Dimension 1 arrays are passed as scalars in Hydra.
         * OP2 assumption is that the user kernel will declare
         * them as scalars.
         * ======================================================
         */
        if ( parallelLoop->getOpDatDimension (i) == 1 )
        {
          SgPntrArrRefExp
              * arrayExpression =
                  buildPntrArrRefExp (
                      buildVarRefExp (
                          sharedIndirectionDeclarations[parallelLoop->getOpDatVariableName (
                              i)]), addExpression2);

          actualParameters->append_expression (arrayExpression);
        }
        else
        {          
          SgAddOp * addExpression3 = buildAddOp (addExpression2, buildIntVal (
              parallelLoop->getOpDatDimension (i)));

          SgSubtractOp * subtractExpression = buildSubtractOp (addExpression3,
              buildIntVal (1));

          SgSubscriptExpression * subscriptExpression =
              new SgSubscriptExpression (RoseHelper::getFileInfo (),
                  addExpression2, subtractExpression, buildIntVal (1));

          subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

          SgPntrArrRefExp
              * arrayExpression =
                  buildPntrArrRefExp (
                      buildVarRefExp (
                          sharedIndirectionDeclarations[parallelLoop->getOpDatVariableName (
                              i)]), subscriptExpression);

          actualParameters->append_expression (arrayExpression);
        }
      }
    }
    else
    {
      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), variableDeclarations->getReference (
              OpenMP::threadBlockOffset));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (addExpression1,
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      
      /*
       * ======================================================
       * Dimension 1 arrays are passed as scalars in Hydra.
       * OP2 assumption is that the user kernel will declare
       * them as scalars.
       * ======================================================
       */
      if ( parallelLoop->getOpDatDimension (i) == 1 )
      {
        Debug::getInstance ()->debugMessage ("Direct opdat with dim 1",
          Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
        
        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatName (i)),
            multiplyExpression);

        actualParameters->append_expression (arrayExpression);
      }
      else
      {
        SgAddOp * addExpression2 = buildAddOp (multiplyExpression, buildIntVal (
            parallelLoop->getOpDatDimension (i)));
        
        SgSubtractOp * subtractExpression = buildSubtractOp (addExpression2,
            buildIntVal (1));

        SgSubscriptExpression * subsricptExpression = new SgSubscriptExpression (
            RoseHelper::getFileInfo (), multiplyExpression, subtractExpression,
            buildIntVal (1));

        subsricptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatName (i)),
            subsricptExpression);

        actualParameters->append_expression (arrayExpression);
      }
    }
  }

  SgFunctionSymbol * userSubroutineSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine (
          userSubroutine->getSubroutineName (), subroutineScope);

  SgFunctionCallExp * userSubroutineCall = buildFunctionCallExp (
      userSubroutineSymbol, actualParameters);

  return buildExprStatement (userSubroutineCall);
}

SgBasicBlock *
FortranOpenMPKernelSubroutineIndirectLoop::createStageOutIncrementedOpDatStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to stage out incremented OP_DATs after kernel execution",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
          getIterationCounterVariableName (1)),
          variableDeclarations->getReference (OpenMP::threadBlockOffset));

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getGlobalToLocalMappingName (i)),
          addExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          variableDeclarations->getReference (getIncrementAccessMapName (i)),
          arrayExpression);

      appendStatement (assignmentStatement, block);
    }
  }

  SgBasicBlock * ifBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          variableDeclarations->getReference (getIncrementAccessMapName (i)),
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), multiplyExpression);

      SgAddOp * addExpression2 = buildAddOp (buildIntVal (1), addExpression1);

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          buildVarRefExp (
              sharedIndirectionDeclarations[parallelLoop->getOpDatVariableName (
                  i)]), addExpression2);

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
          buildVarRefExp (
              sharedIndirectionDeclarations[parallelLoop->getOpDatVariableName (
                  i)]), addExpression2);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgAddOp * addExpression3 =
          buildAddOp (arrayExpression2, arrayExpression3);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression1, addExpression3);

      appendStatement (assignmentStatement, innerLoopBody);

      SgAssignOp * innerLoopLowerBound = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgSubtractOp * innerLoopUpperBound = buildSubtractOp (buildIntVal (
          parallelLoop->getOpDatDimension (i)), buildIntVal (1));

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopLowerBound, innerLoopUpperBound, buildIntVal (1),
              innerLoopBody);

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
FortranOpenMPKernelSubroutineIndirectLoop::createIncrementedOpDatPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise local variables of incremented OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      SgBasicBlock * loopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression, buildIntVal (0));

      appendStatement (assignmentStatement, loopBody);

      SgAssignOp * lowerBound = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgSubtractOp * upperBound = buildSubtractOp (buildIntVal (
          parallelLoop->getOpDatDimension (i)), buildIntVal (1));

      SgFortranDo * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              lowerBound, upperBound, buildIntVal (1), loopBody);

      appendStatement (loopStatement, block);
    }
  }

  return block;
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
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

    appendStatement (createIncrementedOpDatPrologueStatements (), ifBody);

    appendStatement (createUserSubroutineCallStatement (), ifBody);

    SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
        getIterationCounterVariableName (1)),
        variableDeclarations->getReference (OpenMP::threadBlockOffset));

    SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
        variableDeclarations->getReference (getThreadColourArrayName ()),
        addExpression1);

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

    appendStatement (createStageOutIncrementedOpDatStatements (), loopBody);

    SgAssignOp * lowerBound =
        buildAssignOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)), buildIntVal (0));

    SgSubtractOp * upperBound = buildSubtractOp (
        variableDeclarations->getReference (numberOfActiveThreadsCeiling),
        buildIntVal (1));

    SgFortranDo * loopStatement =
        FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
            lowerBound, upperBound, buildIntVal (1), loopBody);

    appendStatement (loopStatement, subroutineScope);
  }
  else
  {
    appendStatement (createUserSubroutineCallStatement (), loopBody);

    SgAssignOp * lowerBound =
        buildAssignOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)), buildIntVal (0));

    SgSubtractOp * upperBound = buildSubtractOp (
        variableDeclarations->getReference (numberOfActiveThreads),
        buildIntVal (1));

    SgFortranDo * loopStatement =
        FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
            lowerBound, upperBound, buildIntVal (1), loopBody);

    appendStatement (loopStatement, subroutineScope);
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createIncrementedOpDatEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to increment indirect OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
      {
        /*
         * ======================================================
         * Inner loop
         * ======================================================
         */

        SgBasicBlock * innerLoopBody = buildBasicBlock ();

        SgAddOp * addExpression1 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (1));

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (getIndirectOpDatMapName (i)),
            addExpression1);

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression2 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), multiplyExpression1);

        SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatName (i)),
            addExpression2);

        SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatName (i)),
            addExpression2);

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression3 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), multiplyExpression2);

        SgAddOp * addExpression4 = buildAddOp (buildIntVal (1), addExpression3);

        SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
            variableDeclarations->getReference (
                getIndirectOpDatSharedMemoryName (i)), addExpression4);

        SgAddOp * addExpression5 = buildAddOp (arrayExpression2,
            arrayExpression4);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            arrayExpression3, addExpression5);

        appendStatement (assignmentStatement, innerLoopBody);

        SgAssignOp * innerLoopLowerBound = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), buildIntVal (0));

        SgSubtractOp * innerLoopUppperBound = buildSubtractOp (buildIntVal (
            parallelLoop->getOpDatDimension (i)), buildIntVal (1));

        SgFortranDo * innerLoopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                innerLoopLowerBound, innerLoopUppperBound, buildIntVal (1),
                innerLoopBody);

        /*
         * ======================================================
         * Outer loop
         * ======================================================
         */

        SgBasicBlock * outerLoopBody = buildBasicBlock ();

        appendStatement (innerLoopStatement, outerLoopBody);

        SgAssignOp * outerLoopLowerBound = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (0));

        SgSubtractOp * outerLoopUppperBound = buildSubtractOp (
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)),
            buildIntVal (1));

        SgFortranDo * outerLoopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                outerLoopLowerBound, outerLoopUppperBound, buildIntVal (1),
                outerLoopBody);

        appendStatement (outerLoopStatement, subroutineScope);
      }
    }
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createStageInStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to stage in indirect OP_DATs into shared memory",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * Inner loop
         * ======================================================
         */

        SgBasicBlock * innerLoopBody = buildBasicBlock ();

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression1 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), multiplyExpression1);

        SgAddOp * addExpression2 = buildAddOp (addExpression1, buildIntVal (1));

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (
                getIndirectOpDatSharedMemoryName (i)), addExpression2);

        if (parallelLoop->isIncremented (i))
        {
          SgExprStatement * assignmentStatement = buildAssignStatement (
              arrayExpression1, buildIntVal (0));

          appendStatement (assignmentStatement, innerLoopBody);
        }
        else
        {
          SgAddOp * addExpression3 = buildAddOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), buildIntVal (1));

          SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
              variableDeclarations->getReference (getIndirectOpDatMapName (i)),
              addExpression3);

          SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
              arrayExpression2, buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgAddOp * addExpression4 = buildAddOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (2)), multiplyExpression2);

          SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
              variableDeclarations->getReference (getOpDatName (i)),
              addExpression4);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              arrayExpression1, arrayExpression3);

          appendStatement (assignmentStatement, innerLoopBody);
        }

        SgAssignOp * innerLoopLowerBound = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), buildIntVal (0));

        SgSubtractOp * innerLoopUppperBound = buildSubtractOp (buildIntVal (
            parallelLoop->getOpDatDimension (i)), buildIntVal (1));

        SgFortranDo * innerLoopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                innerLoopLowerBound, innerLoopUppperBound, buildIntVal (1),
                innerLoopBody);

        /*
         * ======================================================
         * Outer loop
         * ======================================================
         */

        SgBasicBlock * outerLoopBody = buildBasicBlock ();

        appendStatement (innerLoopStatement, outerLoopBody);

        SgAssignOp * outerLoopLowerBound = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (0));

        SgSubtractOp * outerLoopUppperBound = buildSubtractOp (
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)),
            buildIntVal (1));

        SgFortranDo * outerLoopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                outerLoopLowerBound, outerLoopUppperBound, buildIntVal (1),
                outerLoopBody);

        appendStatement (outerLoopStatement, subroutineScope);
      }
    }
  }
}


void
FortranOpenMPKernelSubroutineIndirectLoop::createStageOutStatements ()
{  
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  
  /*
   * ======================================================
   * First increments
   * ======================================================
   */

  Debug::getInstance ()->debugMessage (
      "Creating statements to stage out indirect OP_DATs for OP_INC",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  
  
  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementedOpDatEpilogueStatements ();
  }
  
  /*
   * ======================================================
   * Then OP_WRITE and OP_RW
   * ======================================================
   */

  Debug::getInstance ()->debugMessage (
      "Creating statements to stage out indirect OP_DATs for OP_WRITE and OP_RW",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i) &&
          (parallelLoop->isReadAndWritten (i) || parallelLoop->isWritten (i)))
      {
        /*
         * ======================================================
         * Inner loop
         * ======================================================
         */

        SgBasicBlock * innerLoopBody = buildBasicBlock ();

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression1 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), multiplyExpression1);

        SgAddOp * addExpression2 = buildAddOp (addExpression1, buildIntVal (1));

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (
                getIndirectOpDatSharedMemoryName (i)), addExpression2);

/*Here*/                
                
          SgAddOp * addExpression3 = buildAddOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), buildIntVal (1));

          SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
              variableDeclarations->getReference (getIndirectOpDatMapName (i)),
              addExpression3);

          SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
              arrayExpression2, buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgAddOp * addExpression4 = buildAddOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (2)), multiplyExpression2);

          SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
              variableDeclarations->getReference (getOpDatName (i)),
              addExpression4);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              arrayExpression3, arrayExpression1);

          appendStatement (assignmentStatement, innerLoopBody);
/*Here*/

        SgAssignOp * innerLoopLowerBound = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), buildIntVal (0));

        SgSubtractOp * innerLoopUppperBound = buildSubtractOp (buildIntVal (
            parallelLoop->getOpDatDimension (i)), buildIntVal (1));

        SgFortranDo * innerLoopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                innerLoopLowerBound, innerLoopUppperBound, buildIntVal (1),
                innerLoopBody);

        /*
         * ======================================================
         * Outer loop
         * ======================================================
         */

        SgBasicBlock * outerLoopBody = buildBasicBlock ();

        appendStatement (innerLoopStatement, outerLoopBody);

        SgAssignOp * outerLoopLowerBound = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (0));

        SgSubtractOp * outerLoopUppperBound = buildSubtractOp (
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)),
            buildIntVal (1));

        SgFortranDo * outerLoopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                outerLoopLowerBound, outerLoopUppperBound, buildIntVal (1),
                outerLoopBody);

        appendStatement (outerLoopStatement, subroutineScope);
      }
    }
  }  
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createInitialiseBytesPerOpDatStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise bytes per OP_DAT",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Initialise round-up variables
   * ======================================================
   */

  bool firstIndirectOpDat = true;
  unsigned int lastIndirectOpDat;

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

        if (firstIndirectOpDat)
        {
          firstIndirectOpDat = false;
          lastIndirectOpDat = i;
        }
        else
        {
          SgMultiplyOp * multiplyExpression = buildMultiplyOp (
              variableDeclarations->getReference (getIndirectOpDatSizeName (
                  lastIndirectOpDat)), buildIntVal (
                  parallelLoop->getOpDatDimension (lastIndirectOpDat)));

          SgExprStatement * assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (getRoundUpVariableName (i)),
              multiplyExpression);

          appendStatement (assignmentStatement, subroutineScope);

          lastIndirectOpDat = i;
        }
      }
    }
  }

  /*
   * ======================================================
   * Initialise number of bytes variables
   * ======================================================
   */

  firstIndirectOpDat = true;

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
        if (firstIndirectOpDat)
        {
          firstIndirectOpDat = false;

          SgExprStatement * assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), buildIntVal (0));

          appendStatement (assignmentStatement, subroutineScope);
        }
        else
        {
          SgAddOp * addExpression = buildAddOp (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  lasti)), variableDeclarations->getReference (
                  getRoundUpVariableName (i)));

          SgExprStatement * assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), addExpression);

          appendStatement (assignmentStatement, subroutineScope);
        }

        lasti = i;
      }
    }
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createInitialiseSharedVariableStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise shared variables of indirect OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const & sharedVariableName = getSharedMemoryDeclarationName (
            parallelLoop->getOpDatBaseType (i),
            parallelLoop->getSizeOfOpDat (i));

        SgSubscriptExpression * subsricptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                variableDeclarations->getReference (
                    getNumberOfBytesVariableName (i)), buildNullExpression (),
                buildIntVal (1));

        subsricptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (sharedVariableName),
            subsricptExpression);

        SgPointerAssignOp * assignExpression = new SgPointerAssignOp (
            RoseHelper::getFileInfo (), variableDeclarations->getReference (
                getIndirectOpDatSharedMemoryName (i)), arrayExpression,
            buildVoidType ());

        assignExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        appendStatement (buildExprStatement (assignExpression), subroutineScope);
      }
    }
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createInitialiseIndirectOpDatSizesStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise sizes of indirect OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (OpenMP::threadBlockID),
            buildIntVal (parallelLoop->getNumberOfDistinctIndirectOpDats ()));

        SgAddOp * addExpression = buildAddOp (buildIntVal (offset),
            multiplyExpression);

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (
                getIndirectOpDatsNumberOfElementsArrayName ()), addExpression);

        SgExprStatement * assignStatement = buildAssignStatement (
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)),
            arrayExpression);

        appendStatement (assignStatement, subroutineScope);

        offset++;
      }
    }
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createInitialiseIndirectOpDatMapsStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise indirect OP_DAT maps",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (OpenMP::threadBlockID),
            buildIntVal (parallelLoop->getNumberOfDistinctIndirectOpDats ()));

        SgAddOp * addExpression = buildAddOp (buildIntVal (offset),
            multiplyExpression);

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (
                getIndirectOpDatsOffsetArrayName ()), addExpression);

        SgSubscriptExpression * subsricptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                arrayExpression1, buildNullExpression (), buildIntVal (1));

        subsricptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        SgPntrArrRefExp * arrayExpression2 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                getLocalToGlobalMappingName (i)), subsricptExpression);

        SgPointerAssignOp * assignExpression = new SgPointerAssignOp (
            RoseHelper::getFileInfo (), variableDeclarations->getReference (
                getIndirectOpDatMapName (i)), arrayExpression2,
            buildVoidType ());

        assignExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        appendStatement (buildExprStatement (assignExpression), subroutineScope);

        offset++;
      }
    }
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createInitialiseIncrementAccessVariablesStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise thread-specific variables needed for incremented OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Statement to initialise ceiling of number of active
   * threads
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (numberOfActiveThreadsCeiling),
      variableDeclarations->getReference (numberOfActiveThreads));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Statement to initialise number of colours
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (
          getNumberOfThreadColoursPerBlockArrayName ()),
      variableDeclarations->getReference (OpenMP::threadBlockID));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (numberOfColours), arrayExpression2);

  appendStatement (assignmentStatement2, subroutineScope);
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createInitialiseThreadVariablesStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialse thread-specific variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Statement to initialise thread-block ID
   * ======================================================
   */

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      blockID), variableDeclarations->getReference (blockOffset));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (getColourToBlockArrayName ()),
      addExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::threadBlockID),
      arrayExpression1);

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Statement to initialise number of active threads
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (
          getNumberOfSetElementsPerBlockArrayName ()),
      variableDeclarations->getReference (OpenMP::threadBlockID));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (numberOfActiveThreads),
      arrayExpression2);

  appendStatement (assignmentStatement2, subroutineScope);

  /*
   * ======================================================
   * Statement to initialise offset into shared memory
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      variableDeclarations->getReference (getOffsetIntoBlockArrayName ()),
      variableDeclarations->getReference (OpenMP::threadBlockID));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::threadBlockOffset),
      arrayExpression3);

  appendStatement (assignmentStatement3, subroutineScope);
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createStatements ()
{
  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createInitialiseThreadVariablesStatements ();

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createInitialiseIncrementAccessVariablesStatements ();
  }

  createInitialiseIndirectOpDatSizesStatements ();

  createInitialiseIndirectOpDatMapsStatements ();

  createInitialiseBytesPerOpDatStatements ();

  createInitialiseSharedVariableStatements ();

  createStageInStatements ();

  createExecutionLoopStatements ();
  
  createStageOutStatements ();
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
{
  using namespace SageBuilder;
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
          "Creating local variables for incremented OP_DAT " + lexical_cast <
              string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      string const variableName1 = getOpDatLocalName (i);

      variableDeclarations->add (variableName1,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName1,
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  parallelLoop->getOpDatBaseType (i), buildIntVal (0),
                  buildIntVal (parallelLoop->getOpDatDimension (i) - 1)),
              subroutineScope));

      string const variableName2 = getIncrementAccessMapName (i);

      variableDeclarations->add (variableName2,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName2, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  variableDeclarations->add (numberOfColours,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfColours, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (numberOfActiveThreadsCeiling,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfActiveThreadsCeiling,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (colour1,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          colour1, FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (colour2,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          colour2, FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createSharedVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::find;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating shared variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <string> autosharedNames;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const & variableName1 = getIndirectOpDatMapName (i);

        variableDeclarations->add (variableName1,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName1, buildPointerType (
                    FortranTypesBuilder::getArray_RankOne (
                        FortranTypesBuilder::getFourByteInteger ())),
                subroutineScope));

        string const & variableName2 = getIndirectOpDatSharedMemoryName (i);

        SgVariableDeclaration * variableDeclaration =
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName2, buildPointerType (
                    FortranTypesBuilder::getArray_RankOne (
                        parallelLoop->getOpDatBaseType (i))), subroutineScope);

        variableDeclarations->add (variableName2, variableDeclaration);

        sharedIndirectionDeclarations[parallelLoop->getOpDatVariableName (i)]
            = variableDeclaration;

        string const & sharedVariableName = getSharedMemoryDeclarationName (
            parallelLoop->getOpDatBaseType (i),
            parallelLoop->getSizeOfOpDat (i));

        if (find (autosharedNames.begin (), autosharedNames.end (),
            sharedVariableName) == autosharedNames.end ())
        {
          Debug::getInstance ()->debugMessage (
              "Creating declaration with name '" + sharedVariableName
                  + "' for OP_DAT '" + parallelLoop->getOpDatVariableName (i)
                  + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          SgSubtractOp * upperBound = buildSubtractOp (buildIntVal (128000),
              buildIntVal (1));

          variableDeclarations->add (
              sharedVariableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  sharedVariableName, FortranTypesBuilder::getArray_RankOne (
                      parallelLoop->getOpDatBaseType (i), 0, upperBound),
                  subroutineScope, 1, TARGET));

          autosharedNames.push_back (sharedVariableName);
        }
      }
    }
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createExecutionLocalVariableDeclarations ()
{
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed to execute kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (OpenMP::threadBlockOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::threadBlockOffset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (OpenMP::threadBlockID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::threadBlockID, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (numberOfActiveThreads,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfActiveThreads, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations ->add (getIterationCounterVariableName (1),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (getIterationCounterVariableName (2),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createIndirectOpDatSizeLocalVariableDeclarations ()
{
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating cardinality argument for indirect OP_DAT "
                + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
            __LINE__);

        string const variableName = getIndirectOpDatSizeName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getFourByteInteger (),
                subroutineScope));
      }
    }
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createRoundUpLocalVariableDeclarations ()
{
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
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
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createNumberOfBytesPerOpDatLocalVariableDeclarations ()
{
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = getNumberOfBytesVariableName (i);

    if (parallelLoop->isIndirect (i))
    {
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
            "Number of bytes declaration NOT needed for OP_DAT "
                + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
            __LINE__);

        variableDeclarations ->add (variableName,
            numberOfBytesDeclarations[parallelLoop->getOpDatVariableName (i)]);
      }
    }
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createExecutionLocalVariableDeclarations ();

  createSharedVariableDeclarations ();

  createNumberOfBytesPerOpDatLocalVariableDeclarations ();

  createRoundUpLocalVariableDeclarations ();

  createIndirectOpDatSizeLocalVariableDeclarations ();

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementAccessLocalVariableDeclarations ();
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating plan function formal parameter declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgAsteriskShapeExp * upperBoundExpression = new SgAsteriskShapeExp (
      RoseHelper::getFileInfo ());

  variableDeclarations->add (
      getIndirectOpDatsNumberOfElementsArrayName (),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getIndirectOpDatsNumberOfElementsArrayName (),
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression), subroutineScope, formalParameters));

  variableDeclarations->add (
      getIndirectOpDatsOffsetArrayName (),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getIndirectOpDatsOffsetArrayName (),
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression), subroutineScope, formalParameters));

  variableDeclarations->add (
      getColourToBlockArrayName (),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getColourToBlockArrayName (),
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression), subroutineScope, formalParameters));

  variableDeclarations->add (
      getOffsetIntoBlockArrayName (),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getOffsetIntoBlockArrayName (),
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression), subroutineScope, formalParameters));

  variableDeclarations->add (
      getNumberOfSetElementsPerBlockArrayName (),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getNumberOfSetElementsPerBlockArrayName (),
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression), subroutineScope, formalParameters));

  variableDeclarations->add (
      getNumberOfThreadColoursPerBlockArrayName (),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getNumberOfThreadColoursPerBlockArrayName (),
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression), subroutineScope, formalParameters));

  variableDeclarations->add (
      getThreadColourArrayName (),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getThreadColourArrayName (),
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
              upperBoundExpression), subroutineScope, formalParameters));

  variableDeclarations->add (
      blockOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          blockOffset, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters));

  variableDeclarations->add (
      blockID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          blockID, FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters));
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    /*
     * ======================================================
     * First treat global variables
     * ======================================================
     */
    
    if ( parallelLoop->isGlobal (i) )
    {
      if ( parallelLoop->isArray (i) )
      {
           variableDeclarations->add (
              getOpDatName (i),
              FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                  getOpDatName (i),
                  FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                      parallelLoop->getOpDatBaseType (i), buildIntVal (0),
                      buildIntVal (parallelLoop->getOpDatDimension (i) - 1)),
                  subroutineScope, formalParameters, 0));
      }
      else
      {
      variableDeclarations->add (
          getOpDatName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              getOpDatName (i), parallelLoop->getOpDatBaseType (i), subroutineScope, formalParameters));
      }
    }

    else if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = getOpDatName (i);

      SgAsteriskShapeExp * upperBoundExpression = new SgAsteriskShapeExp (
          RoseHelper::getFileInfo ());

      variableDeclarations->add (
          variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName,
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  parallelLoop->getOpDatBaseType (i), buildIntVal (0),
                  upperBoundExpression), subroutineScope, formalParameters));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const & variableName = getLocalToGlobalMappingName (i);

        variableDeclarations->add (
            variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName,
                FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                    FortranTypesBuilder::getFourByteInteger (),
                    buildIntVal (0), buildNullExpression ()), subroutineScope,
                formalParameters, 1, TARGET));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      string const & variableName = getGlobalToLocalMappingName (i);

      SgAsteriskShapeExp * upperBoundExpression = new SgAsteriskShapeExp (
          RoseHelper::getFileInfo ());

      variableDeclarations->add (
          variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName,
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  FortranTypesBuilder::getTwoByteInteger (), buildIntVal (0),
                  upperBoundExpression), subroutineScope, formalParameters));
    }
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
}

FortranOpenMPKernelSubroutineIndirectLoop::FortranOpenMPKernelSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    Subroutine <SgProcedureHeaderStatement> * userSubroutine,
    FortranParallelLoop * parallelLoop) :
  FortranOpenMPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "OpenMP kernel subroutine creation for indirect loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
