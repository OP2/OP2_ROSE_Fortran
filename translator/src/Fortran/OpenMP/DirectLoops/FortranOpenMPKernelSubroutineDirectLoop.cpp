


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


#include <FortranOpenMPKernelSubroutineDirectLoop.h>
#include <FortranParallelLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CompilerGeneratedNames.h>
#include <OpenMP.h>

SgStatement *
FortranOpenMPKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage ("Creating call to user kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression2 = buildAddOp (multiplyExpression2,
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgSubtractOp * subtractExpression2 = buildSubtractOp (addExpression2,
            buildIntVal (1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                multiplyExpression1, subtractExpression2, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            RoseHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatName (i)),
            buildExprListExp (arraySubscriptExpression));

        actualParameters->append_expression (parameterExpression);
      }
      else
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (getOpDatName (i)));
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

void
FortranOpenMPKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating do loop statements for thread slice", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgAssignOp * initializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      variableDeclarations->getReference (OpenMP::sliceStart));

  SgSubtractOp * upperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (OpenMP::sliceEnd), buildIntVal (1));

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (createUserSubroutineCallStatement (), loopBody);

  SgFortranDo * doStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  appendStatement (doStatement, subroutineScope);
}

void
FortranOpenMPKernelSubroutineDirectLoop::createStatements ()
{
  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createExecutionLoopStatements ();
}

void
FortranOpenMPKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  using namespace LoopVariableNames;

  variableDeclarations->add (getIterationCounterVariableName (1),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranOpenMPKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "OpenMP kernel subroutine creation for direct loop",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgArrayType * arrayType =
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              parallelLoop->getOpDatBaseType (i), buildIntVal (0),
              new SgAsteriskShapeExp (RoseHelper::getFileInfo ()));

      variableDeclarations->add (
          getOpDatName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              getOpDatName (i), arrayType, subroutineScope, formalParameters));
    }
  }
}

void
FortranOpenMPKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  createOpDatFormalParameterDeclarations ();

  variableDeclarations->add (
      OpenMP::sliceStart,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OpenMP::sliceStart, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters));

  variableDeclarations->add (
      OpenMP::sliceEnd,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OpenMP::sliceEnd, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters));
}

FortranOpenMPKernelSubroutineDirectLoop::FortranOpenMPKernelSubroutineDirectLoop (
    SgScopeStatement * moduleScope,
    Subroutine <SgProcedureHeaderStatement> * userSubroutine,
    FortranParallelLoop * parallelLoop) :
  FortranOpenMPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "OpenMP kernel subroutine creation for direct loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
