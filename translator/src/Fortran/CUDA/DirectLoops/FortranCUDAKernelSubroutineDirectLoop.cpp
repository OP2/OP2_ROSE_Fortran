


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


#include <FortranCUDAKernelSubroutineDirectLoop.h>
#include <FortranCUDAUserSubroutine.h>
#include <FortranCUDAModuleDeclarations.h>
#include <FortranCUDAOpDatCardinalitiesDeclaration.h>
#include <FortranReductionSubroutines.h>
#include <FortranParallelLoop.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CompilerGeneratedNames.h>
#include <CUDA.h>

SgStatement *
FortranCUDAKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
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

    if (parallelLoop->isDirect (i))
    {
      Debug::getInstance ()->debugMessage ("Direct argument",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->getOpDatDimension (i) == 1)
      {
        parameterExpression = buildPntrArrRefExp (
            moduleDeclarations->getDeclarations ()->getReference (
            getOpDatName (i) + deviceString + parallelLoop->getUserSubroutineName () + postfixName),
            buildAddOp ( variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (1)));
      }
      else
      {
        parameterExpression = variableDeclarations->getReference (
            getOpDatLocalName (i));
      }
    }
    else if (parallelLoop->isReductionRequired (i))
    {
      Debug::getInstance ()->debugMessage ("Reduction argument",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

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

        string const variableName = getOpDatCardinalityName (i);

        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (opDatCardinalities),
            buildOpaqueVarRefExp (variableName, subroutineScope));

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

SgFortranDo *
FortranCUDAKernelSubroutineDirectLoop::createStageInFromDeviceMemoryToSharedMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using std::string;

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
  
  string const autosharedVariableName = getSharedMemoryDeclarationName (
      parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
      parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      getSharedMemoryOffsetDeclarationName (parallelLoop->getOpDatBaseType (
          OP_DAT_ArgumentGroup), parallelLoop->getSizeOfOpDat (
          OP_DAT_ArgumentGroup));

  SgAssignOp * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (0));

  SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
      opDatDimensions), dimensionsDeclaration->getOpDatDimensionField (
      OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      variableDeclarations->getReference (numberOfActiveThreads));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      threadID), multiplyExpression1);

  SgAddOp * addExpression2 = buildAddOp (variableDeclarations->getReference (
      autosharedOffsetVariableName), addExpression1);

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      variableDeclarations->getReference (numberOfActiveThreads));

  SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (
      variableDeclarations->getReference (localOffset), dotExpression);

  SgAddOp * addExpression3 = buildAddOp (multiplyExpression2,
      multiplyExpression3);

  SgAddOp * addExpression4 = buildAddOp (variableDeclarations->getReference (
      threadID), addExpression3);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression2);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      moduleDeclarations->getDeclarations ()->getReference (
      getOpDatName (OP_DAT_ArgumentGroup) + deviceString + parallelLoop->getUserSubroutineName () + postfixName),
      buildAddOp (addExpression4, buildIntVal (1)));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, arrayExpression2);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initialisationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  return loopStatement;
}

SgFortranDo *
FortranCUDAKernelSubroutineDirectLoop::createStageInFromSharedMemoryToLocalMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using std::string;

  string const autosharedVariableName = getSharedMemoryDeclarationName (
      parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
      parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      getSharedMemoryOffsetDeclarationName (parallelLoop->getOpDatBaseType (
          OP_DAT_ArgumentGroup), parallelLoop->getSizeOfOpDat (
          OP_DAT_ArgumentGroup));

  SgAssignOp * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (0));

  SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
      opDatDimensions), dimensionsDeclaration->getOpDatDimensionField (
      OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (
      variableDeclarations->getReference (threadID), dotExpression);

  SgAddOp * addExpression5 = buildAddOp (variableDeclarations->getReference (
      getIterationCounterVariableName (2)), multiplyExpression4);

  SgAddOp * addExpression6 = buildAddOp (variableDeclarations->getReference (
      autosharedOffsetVariableName), addExpression5);

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      variableDeclarations->getReference (getOpDatLocalName (
          OP_DAT_ArgumentGroup)), variableDeclarations->getReference (
          getIterationCounterVariableName (2)));

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression6);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      arrayExpression3, arrayExpression4);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement2);

  SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initialisationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  return loopStatement;
}

SgFortranDo *
FortranCUDAKernelSubroutineDirectLoop::createStageOutFromSharedMemoryToDeviceMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using std::string;

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);  
  
  string const autosharedVariableName = getSharedMemoryDeclarationName (
      parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
      parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      getSharedMemoryOffsetDeclarationName (parallelLoop->getOpDatBaseType (
          OP_DAT_ArgumentGroup), parallelLoop->getSizeOfOpDat (
          OP_DAT_ArgumentGroup));

  SgExpression * loopInitializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (0));

  SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
      opDatDimensions), dimensionsDeclaration->getOpDatDimensionField (
      OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      variableDeclarations->getReference (numberOfActiveThreads));

  SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (
      variableDeclarations->getReference (localOffset), dotExpression);

  SgAddOp * addExpression3 = buildAddOp (multiplyExpression2,
      multiplyExpression3);

  SgAddOp * addExpression4 = buildAddOp (variableDeclarations->getReference (
      threadID), addExpression3);

  SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      variableDeclarations->getReference (numberOfActiveThreads));

  SgAddOp * addExpression5 = buildAddOp (variableDeclarations->getReference (
      threadID), multiplyExpression4);

  SgAddOp * addExpression6 = buildAddOp (variableDeclarations->getReference (
      autosharedOffsetVariableName), addExpression5);

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      moduleDeclarations->getDeclarations ()->getReference (
      getOpDatName (OP_DAT_ArgumentGroup) + deviceString + parallelLoop->getUserSubroutineName () + postfixName),
      buildAddOp (addExpression4, buildIntVal (1)));

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression6);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      arrayExpression3, arrayExpression4);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement2);

  SgExpression * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          loopInitializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  return loopStatement;
}

SgFortranDo *
FortranCUDAKernelSubroutineDirectLoop::createStageOutFromLocalMemoryToSharedMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using std::string;

  string const autosharedVariableName = getSharedMemoryDeclarationName (
      parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
      parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      getSharedMemoryOffsetDeclarationName (parallelLoop->getOpDatBaseType (
          OP_DAT_ArgumentGroup), parallelLoop->getSizeOfOpDat (
          OP_DAT_ArgumentGroup));

  SgExpression * loopInitializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (0));

  SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
      opDatDimensions), dimensionsDeclaration->getOpDatDimensionField (
      OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
      variableDeclarations->getReference (threadID), dotExpression);

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      getIterationCounterVariableName (2)), multiplyExpression1);

  SgAddOp * addExpression2 = buildAddOp (variableDeclarations->getReference (
      autosharedOffsetVariableName), addExpression1);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (getOpDatLocalName (
          OP_DAT_ArgumentGroup)), variableDeclarations->getReference (
          getIterationCounterVariableName (2)));

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression2);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression2, arrayExpression1);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgExpression * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          loopInitializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  return loopStatement;
}

void
FortranCUDAKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating execution loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgSubtractOp * subtractExpression1 = buildSubtractOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      variableDeclarations->getReference (threadID));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (localOffset), subtractExpression1);

  appendStatement (assignmentStatement1, loopBody);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("min", subroutineScope);

  SgSubtractOp * subtractExpression2 = buildSubtractOp (
      variableDeclarations->getReference (setSize),
      variableDeclarations->getReference (localOffset));

  SgExprListExp * actualParameters = buildExprListExp (
      variableDeclarations->getReference (warpSize), subtractExpression2);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (numberOfActiveThreads), functionCall);

  appendStatement (assignmentStatement2, loopBody);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDirect (i) && parallelLoop->isWritten (i) == false
        && parallelLoop->getOpDatDimension (i) > 1)
    {
      Debug::getInstance ()->debugMessage (
          "Creating statements to stage in from device memory to shared memory for OP_DAT "
              + lexical_cast <string> (i), Debug::OUTER_LOOP_LEVEL, __FILE__,
          __LINE__);

      appendStatement (
          createStageInFromDeviceMemoryToSharedMemoryStatements (i), loopBody);

      Debug::getInstance ()->debugMessage (
          "Creating statements to stage in from shared memory to local memory for OP_DAT "
              + lexical_cast <string> (i), Debug::OUTER_LOOP_LEVEL, __FILE__,
          __LINE__);

      appendStatement (
          createStageInFromSharedMemoryToLocalMemoryStatements (i), loopBody);
    }
  }

  appendStatement (createUserSubroutineCallStatement (), loopBody);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDirect (i) && parallelLoop->isRead (i) == false
        && parallelLoop->getOpDatDimension (i) > 1)
    {
      Debug::getInstance ()->debugMessage (
          "Creating statements to stage out from local memory to shared memory for OP_DAT "
              + lexical_cast <string> (i), Debug::OUTER_LOOP_LEVEL, __FILE__,
          __LINE__);

      appendStatement (
          createStageOutFromLocalMemoryToSharedMemoryStatements (i), loopBody);

      Debug::getInstance ()->debugMessage (
          "Creating statements to stage out from shared memory to device memory for OP_DAT "
              + lexical_cast <string> (i), Debug::OUTER_LOOP_LEVEL, __FILE__,
          __LINE__);

      appendStatement (createStageOutFromSharedMemoryToDeviceMemoryStatements (
          i), loopBody);
    }
  }

  SgSubtractOp * subtractExpression3 = buildSubtractOp (CUDA::getBlockId (
      BLOCK_X, subroutineScope), buildIntVal (1));

  SgMultiplyOp * multiplyExpression = buildMultiplyOp (subtractExpression3,
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

  SgSubtractOp * subtractExpression4 = buildSubtractOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgAddOp * addExpression =
      buildAddOp (subtractExpression4, multiplyExpression);

  SgExpression * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      addExpression);

  SgMultiplyOp * strideExpression = buildMultiplyOp (
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
      CUDA::getGridDimension (BLOCK_X, subroutineScope));

  SgExpression * upperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (setSize), buildIntVal (1));

  SgFortranDo * fortranDoStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initialisationExpression, upperBoundExpression, strideExpression,
          loopBody);

  appendStatement (fortranDoStatement, subroutineScope);
}

void
FortranCUDAKernelSubroutineDirectLoop::createInitialiseOffsetIntoCUDASharedVariableStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::find;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating initialisation statements for offset in CUDA shared variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <string> autosharedOffsetNames;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i) && parallelLoop->getOpDatDimension (i) > 1)
      {
        string const autosharedOffsetVariableName =
            getSharedMemoryOffsetDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        if (find (autosharedOffsetNames.begin (), autosharedOffsetNames.end (),
            autosharedOffsetVariableName) == autosharedOffsetNames.end ())
        {
          autosharedOffsetNames.push_back (autosharedOffsetVariableName);

          SgSubtractOp * subtractExpression = buildSubtractOp (
              CUDA::getThreadId (THREAD_X, subroutineScope), buildIntVal (1));

          SgDivideOp * divideExpression1 = buildDivideOp (subtractExpression,
              variableDeclarations->getReference (warpSize));

          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
              variableDeclarations->getReference (sharedMemoryOffset),
              divideExpression1);

          SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression1,
              buildIntVal (parallelLoop->getSizeOfOpDat (i)));

          SgExprStatement * assignmentStatement =
              buildAssignStatement (variableDeclarations->getReference (
                  autosharedOffsetVariableName), divideExpression2);

          appendStatement (assignmentStatement, subroutineScope);
        }
      }
    }
  }
}

void
FortranCUDAKernelSubroutineDirectLoop::createThreadIDInitialisationStatement ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating thread ID initialisation statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgSubtractOp * subtractExpression = buildSubtractOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("mod", subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp (subtractExpression,
      variableDeclarations->getReference (warpSize));

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  SgExprStatement * assignmentStatement = buildAssignStatement (
      variableDeclarations->getReference (threadID), functionCall);

  appendStatement (assignmentStatement, subroutineScope);
}

void
FortranCUDAKernelSubroutineDirectLoop::createStatements ()
{
  createThreadIDInitialisationStatement ();

  createInitialiseOffsetIntoCUDASharedVariableStatements ();

  createExecutionLoopStatements ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
FortranCUDAKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createCUDAStageInVariablesVariableDeclarations ();

  createCUDASharedVariableDeclarations ();

  vector <string> fourByteIntegers;

  fourByteIntegers.push_back (getIterationCounterVariableName (1));
  fourByteIntegers.push_back (getIterationCounterVariableName (2));
  fourByteIntegers.push_back (numberOfActiveThreads);
  fourByteIntegers.push_back (localOffset);
  fourByteIntegers.push_back (threadID);

  for (vector <string>::iterator it = fourByteIntegers.begin (); it
      != fourByteIntegers.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }
}

void
FortranCUDAKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
/* Carlo: removed declarations of op_dats, left op_gbl variables */

  using namespace SageBuilder;
  using namespace ReductionVariableNames;
  using namespace OP2VariableNames;
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

        variableDeclarations->add (
            variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, FortranTypesBuilder::getArray_RankOne (
                    parallelLoop->getOpDatBaseType (i)), subroutineScope,
                formalParameters, 1, CUDA_DEVICE));
      }
      else if (parallelLoop->isDirect (i))
      {
	/*
        string const & variableName = getOpDatName (i);

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
}

void
FortranCUDAKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * OP_DAT dimensions
   * ======================================================
   */

  variableDeclarations->add (
      opDatDimensions,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opDatDimensions, dimensionsDeclaration->getType (), subroutineScope,
          formalParameters, 1, CUDA_DEVICE));

  /*
   * ======================================================
   * OP_DAT cardinalities
   * ======================================================
   */

  variableDeclarations->add (
      opDatCardinalities,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opDatCardinalities, cardinalitiesDeclaration->getType (),
          subroutineScope, formalParameters, 1, CUDA_DEVICE));

  createOpDatFormalParameterDeclarations ();

  /*
   * ======================================================
   * OP_SET size
   * ======================================================
   */

  variableDeclarations->add (
      setSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          setSize, FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Warp size
   * ======================================================
   */

  variableDeclarations->add (
      warpSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          warpSize, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Offset into shared memory
   * ======================================================
   */

  variableDeclarations->add (
      sharedMemoryOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          sharedMemoryOffset, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters, 1, VALUE));
}

FortranCUDAKernelSubroutineDirectLoop::FortranCUDAKernelSubroutineDirectLoop (
    SgScopeStatement * moduleScope, FortranCUDAUserSubroutine * userSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranReductionSubroutines * reductionSubroutines,
    FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration,
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
