


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


#include "CPPOpenCLKernelSubroutineIndirectLoop.h"
#include "CPPOpenCLUserSubroutine.h"
#include "CPPParallelLoop.h"
#include "RoseHelper.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "OpenCL.h"
#include "OP2.h"
#include "Debug.h"
#include <boost/lexical_cast.hpp>

SgStatement *
CPPOpenCLKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
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

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
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
      else
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with read/read-write/write access",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        SgAddOp * addExpression1 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (sharedMemoryOffset));

        SgPntrArrRefExp * arrayExpression1 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                getGlobalToLocalMappingName (i)), addExpression1);

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        parameterExpression = buildAddOp (variableDeclarations->getReference (
            getIndirectOpDatSharedMemoryName (i)), multiplyExpression1);
      }
    }
    else if (parallelLoop->isDirect (i))
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), variableDeclarations->getReference (sharedMemoryOffset));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (addExpression1,
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      parameterExpression = buildAddOp (variableDeclarations->getReference (
          getOpDatName (i)), multiplyExpression1);
    }
    else if (parallelLoop->isReductionRequired (i))
    {
      Debug::getInstance ()->debugMessage ("Reduction argument",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      parameterExpression = variableDeclarations->getReference (
          getOpDatLocalName (i));
    }
    else
    {
      Debug::getInstance ()->debugMessage ("Global with read access",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->isPointer (i) || parallelLoop->isArray (i))
      {
        parameterExpression = variableDeclarations->getReference (
            getOpDatLocalName (i));
      }
      else
      {
        parameterExpression = variableDeclarations->getReference (getOpDatName (
            i));
      }
    }

    ROSE_ASSERT (parameterExpression != NULL);

    actualParameters->append_expression (parameterExpression);
  }

  /*
   * ======================================================
   * OP_DECL_CONST parameters
   * ======================================================
   */

  for (vector <string>::const_iterator it =
      ((CPPUserSubroutine *) userSubroutine)->firstOpConstReference (); it
      != ((CPPUserSubroutine *) userSubroutine)->lastOpConstReference (); ++it)
  {
    actualParameters->append_expression (variableDeclarations->getReference (
        *it));
  }

  return buildFunctionCallStmt (userSubroutine->getSubroutineName (),
      buildVoidType (), actualParameters, subroutineScope);
}

SgBasicBlock *
CPPOpenCLKernelSubroutineIndirectLoop::createIncrementAndWriteAccessEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating increment and write access epilogue statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (i)
            || parallelLoop->isIncremented (i))
        {
          /*
           * ======================================================
           * For loop body
           * ======================================================
           */

          SgBasicBlock * loopBody = buildBasicBlock ();

          SgDivideOp * divideExpression1 = buildDivideOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgPntrArrRefExp * arrayExpression1a = buildPntrArrRefExp (
              variableDeclarations->getReference (getIndirectOpDatMapName (i)),
              divideExpression1);

          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
              arrayExpression1a, buildIntVal (parallelLoop->getOpDatDimension (
                  i)));

          SgModOp * modulusExpression1 = buildModOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgAddOp * addExpression1 = buildAddOp (modulusExpression1,
              multiplyExpression1);

          SgPntrArrRefExp * arrayExpression1b = buildPntrArrRefExp (
              variableDeclarations->getReference (getOpDatName (i)),
              addExpression1);

          SgPntrArrRefExp * arrayExpression1c = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  getIndirectOpDatSharedMemoryName (i)),
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          if (parallelLoop->isIncremented (i))
          {
            SgPlusAssignOp * assignmentStatement1 = buildPlusAssignOp (
                arrayExpression1b, arrayExpression1c);

            appendStatement (buildExprStatement (assignmentStatement1),
                loopBody);
          }
          else
          {
            SgExprStatement * assignmentStatement1 = buildAssignStatement (
                arrayExpression1b, arrayExpression1c);

            appendStatement (assignmentStatement1, loopBody);
          }

          /*
           * ======================================================
           * For loop statement
           * ======================================================
           */

          SgExprStatement * initialisationExpression = buildAssignStatement (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)),
              OpenCL::getLocalWorkItemIDCallStatement (subroutineScope));

          SgMultiplyOp * multiplyExpression =
              buildMultiplyOp (variableDeclarations->getReference (
                  getIndirectOpDatSizeName (i)), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgLessThanOp * upperBoundExpression = buildLessThanOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), multiplyExpression);

          SgPlusAssignOp * strideExpression = buildPlusAssignOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)),
              OpenCL::getWorkGroupIDCallStatement (subroutineScope));

          SgForStatement * forStatement = buildForStatement (
              initialisationExpression, buildExprStatement (
                  upperBoundExpression), strideExpression, loopBody);

          appendStatement (forStatement, block);
        }
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPOpenCLKernelSubroutineIndirectLoop::createStageOutFromLocalMemoryToSharedMemoryStatements ()
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

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), variableDeclarations->getReference (sharedMemoryOffset));

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (getGlobalToLocalMappingName (i)),
          addExpression1);

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgAddOp * addExpression2 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), multiplyExpression1);

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
          variableDeclarations->getReference (getIndirectOpDatSharedMemoryName (
              i)), addExpression2);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgPlusAssignOp * assignmentStatement = buildPlusAssignOp (
          arrayExpression2, arrayExpression3);

      appendStatement (buildExprStatement (assignmentStatement), innerLoopBody);

      SgExprStatement * innerLoopInitialisationExpression =
          buildAssignStatement (variableDeclarations->getReference (
              getIterationCounterVariableName (2)), buildIntVal (0));

      SgLessThanOp * innerLoopUpperBoundExpression = buildLessThanOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgPlusPlusOp * innerLoopstrideExpression = buildPlusPlusOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgForStatement * innerLoopStatement = buildForStatement (
          innerLoopInitialisationExpression, buildExprStatement (
              innerLoopUpperBoundExpression), innerLoopstrideExpression,
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

  appendStatement (buildExprStatement (
      OpenCL::createWorkItemsSynchronisationCallStatement (subroutineScope)),
      loopBody);

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (colour1), buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (colour1),
      variableDeclarations->getReference (numberOfColours));

  SgPlusPlusOp * strideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (colour1));

  SgForStatement * outerLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (outerLoopStatement, block);

  return block;
}

SgBasicBlock *
CPPOpenCLKernelSubroutineIndirectLoop::createInitialiseIncrementAccessStatements ()
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

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
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

      if (isSgTypeFloat (parallelLoop->getOpDatBaseType (i)))
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            arrayExpression, buildFloatVal (0));

        appendStatement (assignmentStatement, loopBody);
      }
      else if (isSgTypeDouble (parallelLoop->getOpDatBaseType (i)))
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            arrayExpression, buildDoubleVal (0));

        appendStatement (assignmentStatement, loopBody);
      }
      else
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            arrayExpression, buildIntVal (0));

        appendStatement (assignmentStatement, loopBody);
      }

      SgExprStatement * initialisationExpression = buildAssignStatement (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgLessThanOp * upperBoundExpression = buildLessThanOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgPlusPlusOp * strideExpression = buildPlusPlusOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgForStatement * forLoopStatement = buildForStatement (
          initialisationExpression, buildExprStatement (upperBoundExpression),
          strideExpression, loopBody);

      appendStatement (forLoopStatement, block);
    }
  }

  return block;
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
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

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      OpenCL::getLocalWorkItemIDCallStatement (subroutineScope));

  SgPlusAssignOp * strideExpression = buildPlusAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      OpenCL::getLocalWorkGroupSizeCallStatement (subroutineScope));

  SgLessThanOp * upperBoundExpression;

  if (parallelLoop->hasIncrementedOpDats ())
  {
    SgExprStatement * assignmentStatement1 = buildAssignStatement (
        variableDeclarations->getReference (colour2), buildIntVal (-1));

    appendStatement (assignmentStatement1, loopBody);

    SgBasicBlock * ifBody = buildBasicBlock ();

    appendStatementList (
        createInitialiseIncrementAccessStatements ()->getStatementList (),
        ifBody);

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

    appendStatementList (
        createStageOutFromLocalMemoryToSharedMemoryStatements ()->getStatementList (),
        loopBody);

    upperBoundExpression = buildLessThanOp (variableDeclarations->getReference (
        getIterationCounterVariableName (1)),
        variableDeclarations->getReference (numberOfActiveThreadsCeiling));
  }
  else
  {
    appendStatement (createUserSubroutineCallStatement (), loopBody);

    upperBoundExpression = buildLessThanOp (variableDeclarations->getReference (
        getIterationCounterVariableName (1)),
        variableDeclarations->getReference (numberOfActiveThreads));
  }

  SgForStatement * forLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (forLoopStatement, subroutineScope);
}

SgBasicBlock *
CPPOpenCLKernelSubroutineIndirectLoop::createInitialiseOpenCLSharedVariablesStatements ()
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

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * For loop body
         * ======================================================
         */

        SgBasicBlock * loopBody = buildBasicBlock ();

        if (parallelLoop->isIncremented (i))
        {
          SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  getIndirectOpDatSharedMemoryName (i)),
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          if (isSgTypeFloat (parallelLoop->getOpDatBaseType (i)))
          {
            SgExprStatement * assignmentStatement = buildAssignStatement (
                arrayExpression, buildFloatVal (0.0));

            appendStatement (assignmentStatement, loopBody);
          }
          else if (isSgTypeDouble (parallelLoop->getOpDatBaseType (i)))
          {
            SgExprStatement * assignmentStatement = buildAssignStatement (
                arrayExpression, buildDoubleVal (0));

            appendStatement (assignmentStatement, loopBody);
          }
          else
          {
            SgExprStatement * assignmentStatement = buildAssignStatement (
                arrayExpression, buildIntVal (0));

            appendStatement (assignmentStatement, loopBody);
          }
        }
        else
        {
          SgDivideOp * divideExpression = buildDivideOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
              variableDeclarations->getReference (getIndirectOpDatMapName (i)),
              divideExpression);

          SgMultiplyOp * multiplyExpression = buildMultiplyOp (
              arrayExpression1, buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgModOp * modulusExpression = buildModOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgAddOp * addExpression = buildAddOp (modulusExpression,
              multiplyExpression);

          SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
              variableDeclarations->getReference (getOpDatName (i)),
              addExpression);

          SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  getIndirectOpDatSharedMemoryName (i)),
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          SgExprStatement * assignmentStatement = buildAssignStatement (
              arrayExpression3, arrayExpression2);

          appendStatement (assignmentStatement, loopBody);
        }

        /*
         * ======================================================
         * For loop statement
         * ======================================================
         */

        SgExprStatement * initialisationExpression = buildAssignStatement (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            OpenCL::getLocalWorkItemIDCallStatement (subroutineScope));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)),
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgLessThanOp * upperBoundExpression = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), multiplyExpression);

        SgPlusAssignOp * strideExpression = buildPlusAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            OpenCL::getLocalWorkGroupSizeCallStatement (subroutineScope));

        SgForStatement * forStatement = buildForStatement (
            initialisationExpression,
            buildExprStatement (upperBoundExpression), strideExpression,
            loopBody);

        appendStatement (forStatement, block);
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPOpenCLKernelSubroutineIndirectLoop::createSetIndirectionMapPointerStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to set indirection map pointers",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * New statement
         * ======================================================
         */

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
            variableDeclarations->getReference (blockID), buildIntVal (
                parallelLoop->getNumberOfDistinctIndirectOpDats ()));

        SgAddOp * addExpression1a = buildAddOp (buildIntVal (offset),
            multiplyExpression1);

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (pindOffs), addExpression1a);

        SgAddOp * addExpression1b =
            buildAddOp (variableDeclarations->getReference (
                getLocalToGlobalMappingName (i)), arrayExpression1);

        SgExprStatement * assignmentStatement1 = buildAssignStatement (
            variableDeclarations->getReference (getIndirectOpDatMapName (i)),
            addExpression1b);

        appendStatement (assignmentStatement1, block);

        ++offset;
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPOpenCLKernelSubroutineIndirectLoop::createSetOpDatSharedMemoryPointerStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT shared memory pointers",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (nbytes), buildIntVal (0));

  appendStatement (assignmentStatement1, block);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  unsigned int indirectOpDatCounter = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * New statement
         * ======================================================
         */

        SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
            variableDeclarations->getReference (getSharedMemoryDeclarationName (
                parallelLoop->getUserSubroutineName ())),
            variableDeclarations->getReference (nbytes));

        SgAddressOfOp * addressOfExpression2 = buildAddressOfOp (
            arrayExpression2);

        SgCastExp * castExpression2 = buildCastExp (addressOfExpression2,
            buildPointerType (parallelLoop->getOpDatBaseType (i)));

        SgExprStatement * assignmentStatement2 = buildAssignStatement (
            variableDeclarations->getReference (
                getIndirectOpDatSharedMemoryName (i)), castExpression2);

        appendStatement (assignmentStatement2, block);

        /*
         * ======================================================
         * New statement
         * ======================================================
         */

        indirectOpDatCounter++;

        if (indirectOpDatCounter
            < parallelLoop->getNumberOfDistinctIndirectOpDats ())
        {
          SgMultiplyOp * multiplyExpression3a = buildMultiplyOp (buildSizeOfOp (
              parallelLoop->getOpDatBaseType (i)), buildIntVal (
              parallelLoop->getOpDatDimension (i)));

          SgMultiplyOp * multiplyExpression3b =
              buildMultiplyOp (variableDeclarations->getReference (
                  getIndirectOpDatSizeName (i)), multiplyExpression3a);

          SgFunctionCallExp * functionCallExpression3 =
              OP2::Macros::createRoundUpCallStatement (subroutineScope,
                  multiplyExpression3b);

          SgPlusAssignOp * assignmentStatement3 = buildPlusAssignOp (
              variableDeclarations->getReference (nbytes),
              functionCallExpression3);

          appendStatement (buildExprStatement (assignmentStatement3), block);
        }
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPOpenCLKernelSubroutineIndirectLoop::createSetNumberOfIndirectElementsPerBlockStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise sizes of indirect OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
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

        SgStatement * statement = buildAssignStatement (
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)),
            arrayExpression);

        appendStatement (statement, block);

        ++offset;
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPOpenCLKernelSubroutineIndirectLoop::createIncrementAccessThreadZeroStatements ()
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
      OpenCL::getLocalWorkGroupSizeCallStatement (subroutineScope));

  SgExpression * addExpression1 = buildAddOp (buildIntVal (1),
      divideExpression1);

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
      OpenCL::getLocalWorkGroupSizeCallStatement (subroutineScope),
      addExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildOpaqueVarRefExp (numberOfActiveThreadsCeiling, subroutineScope),
      multiplyExpression1);

  appendStatement (assignmentStatement1, block);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (pnthrcol, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgExprStatement * assignmentStatement2 =
      buildAssignStatement (buildOpaqueVarRefExp (numberOfColours,
          subroutineScope), arrayExpression2);

  appendStatement (assignmentStatement2, block);

  return block;
}

SgIfStmt *
CPPOpenCLKernelSubroutineIndirectLoop::createThreadZeroStatements ()
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

  SgAddOp * arrayIndexExpression1 = buildAddOp (
      OpenCL::getGlobalWorkGroupSizeCallStatement (subroutineScope),
      variableDeclarations->getReference (blockOffset));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (pblkMap), arrayIndexExpression1);

  SgStatement * statement1 = buildAssignStatement (
      variableDeclarations->getReference (blockID), arrayExpression1);

  appendStatement (statement1, ifBlock);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (pnelems, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgStatement * statement2 = buildAssignStatement (
      variableDeclarations->getReference (numberOfActiveThreads),
      arrayExpression2);

  appendStatement (statement2, ifBlock);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  if (parallelLoop->hasIncrementedOpDats ())
  {
    appendStatementList (
        createIncrementAccessThreadZeroStatements ()->getStatementList (),
        ifBlock);
  }

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (poffset, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgStatement * statement3 = buildAssignStatement (buildOpaqueVarRefExp (
      sharedMemoryOffset, subroutineScope), arrayExpression3);

  appendStatement (statement3, ifBlock);

  appendStatementList (
      createSetNumberOfIndirectElementsPerBlockStatements ()->getStatementList (),
      ifBlock);

  appendStatementList (
      createSetIndirectionMapPointerStatements ()->getStatementList (), ifBlock);

  appendStatementList (
      createSetOpDatSharedMemoryPointerStatements ()->getStatementList (),
      ifBlock);

  /*
   * ======================================================
   * Add the if statement with a NULL else block
   * ======================================================
   */

  SgEqualityOp * ifGuardExpression = buildEqualityOp (
      OpenCL::getLocalWorkItemIDCallStatement (subroutineScope),
      buildIntVal (0));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  return ifStatement;
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  appendStatement (createThreadZeroStatements (), subroutineScope);

  appendStatement (buildExprStatement (
      OpenCL::createWorkItemsSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  appendStatementList (
      createInitialiseOpenCLSharedVariablesStatements ()->getStatementList (),
      subroutineScope);

  appendStatement (buildExprStatement (
      OpenCL::createWorkItemsSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  createExecutionLoopStatements ();

  appendStatementList (
      createIncrementAndWriteAccessEpilogueStatements ()->getStatementList (),
      subroutineScope);
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
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

  SgVariableDeclaration * variableDeclaration1 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfColours, buildIntType (), subroutineScope);

  variableDeclarations->add (numberOfColours, variableDeclaration1);

  variableDeclaration1->get_declarationModifier ().get_storageModifier ().setOpenclLocal ();

  SgVariableDeclaration * variableDeclaration2 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfActiveThreadsCeiling, buildIntType (), subroutineScope);

  variableDeclaration2->get_declarationModifier ().get_storageModifier ().setOpenclLocal ();

  variableDeclarations->add (numberOfActiveThreadsCeiling, variableDeclaration2);

  variableDeclarations ->add (colour1,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (colour1,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (colour2,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (colour2,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (
      getIterationCounterVariableName (2),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2), buildIntType (), subroutineScope));
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createStageInVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local thread variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      string const & variableName = getOpDatLocalName (i);

      SgVariableDeclaration * variableDeclaration =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildArrayType (parallelLoop->getOpDatBaseType (i),
                  buildIntVal (parallelLoop->getOpDatDimension (i))),
              subroutineScope);

      variableDeclarations ->add (variableName, variableDeclaration);
    }
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createExecutionLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed to execute kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration1 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          sharedMemoryOffset, buildIntType (), subroutineScope);

  variableDeclaration1->get_declarationModifier ().get_storageModifier ().setOpenclLocal ();

  variableDeclarations->add (sharedMemoryOffset, variableDeclaration1);

  SgVariableDeclaration * variableDeclaration2 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfActiveThreads, buildIntType (), subroutineScope);

  variableDeclaration2->get_declarationModifier ().get_storageModifier ().setOpenclLocal ();

  variableDeclarations->add (numberOfActiveThreads, variableDeclaration2);

  SgVariableDeclaration * variableDeclaration3 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nbytes,
          buildIntType (), subroutineScope);

  variableDeclaration3->get_declarationModifier ().get_storageModifier ().setOpenclLocal ();

  variableDeclarations ->add (nbytes, variableDeclaration3);

  variableDeclarations->add (blockID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (blockID,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createStageInVariableDeclarations ();

  createExecutionLocalVariableDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionVariableDeclarations ();
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating shared indirection mapping for OP_DAT " + lexical_cast <
                string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

        string const variableName = getIndirectOpDatMapName (i);

        SgVariableDeclaration * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildPointerType (buildIntType ()),
                subroutineScope);

        variableDeclaration->get_declarationModifier ().get_storageModifier ().setOpenclLocal ();

        variableDeclarations->add (variableName, variableDeclaration);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating indirection size argument for OP_DAT " + lexical_cast <
                string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

        string const variableName = getIndirectOpDatSizeName (i);

        SgVariableDeclaration * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildIntType (), subroutineScope);

        variableDeclaration->get_declarationModifier ().get_storageModifier ().setOpenclLocal ();

        variableDeclarations->add (variableName, variableDeclaration);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating shared memory pointer for OP_DAT "
                + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
            __LINE__);

        string const variableName = getIndirectOpDatSharedMemoryName (i);

        SgVariableDeclaration * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope);

        variableDeclaration->get_declarationModifier ().get_storageModifier ().setOpenclLocal ();

        variableDeclarations->add (variableName, variableDeclaration);

        indirectOpDatSharedMemoryDeclarations[parallelLoop->getOpDatVariableName (
            i)] = variableDeclaration;
      }
    }
    else
    {
      Debug::getInstance ()->debugMessage (
          "New shared memory declaration NOT needed for OP_DAT "
              + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
          __LINE__);

      string const variableName = getIndirectOpDatSharedMemoryName (i);

      variableDeclarations ->add (
          variableName,
          indirectOpDatSharedMemoryDeclarations[parallelLoop->getOpDatVariableName (
              i)]);
    }
  }

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementAccessLocalVariableDeclarations ();
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating plan formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * New variable declaration
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration1 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              pindSizes, buildPointerType (buildIntType ()), subroutineScope,
              formalParameters);

  (*variableDeclaration1->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

  variableDeclarations->add (pindSizes, variableDeclaration1);

  /*
   * ======================================================
   * New variable declaration
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration2 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              pindOffs, buildPointerType (buildIntType ()), subroutineScope,
              formalParameters);

  (*variableDeclaration2->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

  variableDeclarations->add (pindOffs, variableDeclaration2);

  /*
   * ======================================================
   * New variable declaration
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration3 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              pblkMap, buildPointerType (buildIntType ()), subroutineScope,
              formalParameters);

  (*variableDeclaration3->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

  variableDeclarations->add (pblkMap, variableDeclaration3);

  /*
   * ======================================================
   * New variable declaration
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration4 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              poffset, buildPointerType (buildIntType ()), subroutineScope,
              formalParameters);

  (*variableDeclaration4->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

  variableDeclarations->add (poffset, variableDeclaration4);

  /*
   * ======================================================
   * New variable declaration
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration5 =

          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              pnelems, buildPointerType (buildIntType ()), subroutineScope,
              formalParameters);

  (*variableDeclaration5->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

  variableDeclarations->add (pnelems, variableDeclaration5);

  /*
   * ======================================================
   * New variable declaration
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration6 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              pnthrcol, buildPointerType (buildIntType ()), subroutineScope,
              formalParameters);

  (*variableDeclaration6->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

  variableDeclarations->add (pnthrcol, variableDeclaration6);

  /*
   * ======================================================
   * New variable declaration
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration7 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              pthrcol, buildPointerType (buildIntType ()), subroutineScope,
              formalParameters);

  (*variableDeclaration7->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

  variableDeclarations->add (pthrcol, variableDeclaration7);

  /*
   * ======================================================
   * New variable declaration
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration8 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              blockOffset, buildIntType (), subroutineScope, formalParameters);

  (*variableDeclaration8->get_variables ().begin ())->get_storageModifier ().setOpenclPrivate ();

  variableDeclarations->add (blockOffset, variableDeclaration8);

  /*
   * ======================================================
   * New variable declaration
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration9 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              getSharedMemoryDeclarationName (
                  parallelLoop->getUserSubroutineName ()), buildIntType (),
              subroutineScope, formalParameters);

  (*variableDeclaration9->get_variables ().begin ())->get_storageModifier ().setOpenclLocal ();

  variableDeclarations->add (getSharedMemoryDeclarationName (
      parallelLoop->getUserSubroutineName ()), variableDeclaration9);
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i))
      {
        string const & variableName = getReductionArrayDeviceName (i);

        SgVariableDeclaration
            * variableDeclaration =
                RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName, buildPointerType (
                        parallelLoop->getOpDatBaseType (i)), subroutineScope,
                    formalParameters);

        (*variableDeclaration->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

        variableDeclarations->add (variableName, variableDeclaration);
      }
      else if (parallelLoop->isIndirect (i) || parallelLoop->isDirect (i))
      {
        string const variableName = getOpDatName (i);

        SgVariableDeclaration
            * variableDeclaration =
                RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName, buildPointerType (
                        parallelLoop->getOpDatBaseType (i)), subroutineScope,
                    formalParameters);

        (*variableDeclaration->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

        variableDeclarations->add (variableName, variableDeclaration);
      }
      else if (parallelLoop->isRead (i))
      {
        string const & variableName = getOpDatName (i);

        if (parallelLoop->isPointer (i))
        {
          SgVariableDeclaration
              * variableDeclaration =
                  RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                      variableName, buildPointerType (
                          parallelLoop->getOpDatBaseType (i)), subroutineScope,
                      formalParameters);

          (*variableDeclaration->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

          variableDeclarations->add (variableName, variableDeclaration);
        }
        else
        {
          SgVariableDeclaration
              * variableDeclaration =
                  RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                      variableName, parallelLoop->getOpDatBaseType (i),
                      subroutineScope, formalParameters);

          (*variableDeclaration->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

          variableDeclarations->add (variableName, variableDeclaration);
        }
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const & variableName = getLocalToGlobalMappingName (i);

        SgVariableDeclaration
            * variableDeclaration =
                RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName, buildPointerType (buildIntType ()),
                    subroutineScope, formalParameters);

        (*variableDeclaration->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

        variableDeclarations->add (variableName, variableDeclaration);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isIndirect (i))
    {
      string const & variableName = getGlobalToLocalMappingName (i);

      SgVariableDeclaration
          * variableDeclaration =
              RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                  variableName, buildPointerType (buildShortType ()),
                  subroutineScope, formalParameters);

      (*variableDeclaration->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

      variableDeclarations->add (variableName, variableDeclaration);
    }
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();

  createOpDeclConstFormalParameterDeclarations ();
}

CPPOpenCLKernelSubroutineIndirectLoop::CPPOpenCLKernelSubroutineIndirectLoop (
    SgScopeStatement * moduleScope, CPPOpenCLUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines,
    CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPOpenCLKernelSubroutine (moduleScope, userSubroutine, parallelLoop,
      reductionSubroutines, declarations)
{
  Debug::getInstance ()->debugMessage ("<Kernel, Indirect, OpenCL>",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
