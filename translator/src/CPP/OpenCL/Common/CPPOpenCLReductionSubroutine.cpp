


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


#include "CPPOpenCLReductionSubroutine.h"
#include "Reduction.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include "OP2.h"
#include "OpenCL.h"

void
CPPOpenCLReductionSubroutine::createThreadZeroReductionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;

  /*
   * ======================================================
   * The increment reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      buildIntVal (0));

  SgPointerDerefExp * pointerDerefExpression1 = buildPointerDerefExp (
      variableDeclarations->getReference (reductionResult));

  SgAddOp * addExpression = buildAddOp (pointerDerefExpression1,
      arrayExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildPointerDerefExp (
          variableDeclarations->getReference (reductionResult)), addExpression);

  SgIntVal * incrementCaseExpression = buildIntVal (INCREMENT);

  SgBasicBlock * incrementBasicBlock = buildBasicBlock ();

  appendStatement (assignmentStatement1, incrementBasicBlock);

  appendStatement (buildBreakStmt (), incrementBasicBlock);

  SgCaseOptionStmt * incrementCaseStatement = buildCaseOptionStmt (
      incrementCaseExpression, incrementBasicBlock);

  /*
   * ======================================================
   * The maximum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      buildIntVal (0));

  SgPointerDerefExp * pointerDerefExpression2 = buildPointerDerefExp (
      variableDeclarations->getReference (reductionResult));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      pointerDerefExpression2, arrayExpression2);

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), buildIntVal (0));

  SgGreaterThanOp * ifGuardExpression1 = buildGreaterThanOp (arrayExpression3,
      pointerDerefExpression2);

  SgIfStmt * maximumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression1, buildBasicBlock (assignmentStatement2));

  SgBasicBlock * maximumBasicBlock = buildBasicBlock ();

  appendStatement (maximumIfStatement, maximumBasicBlock);

  appendStatement (buildBreakStmt (), maximumBasicBlock);

  SgIntVal * maximumCaseExpression = buildIntVal (MAXIMUM);

  SgCaseOptionStmt * maximumCaseStatement = buildCaseOptionStmt (
      maximumCaseExpression, maximumBasicBlock);

  /*
   * ======================================================
   * The minimum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      buildIntVal (0));

  SgPointerDerefExp * pointerDerefExpression3 = buildPointerDerefExp (
      variableDeclarations->getReference (reductionResult));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      pointerDerefExpression3, arrayExpression4);

  SgPntrArrRefExp * arrayExpression5 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), buildIntVal (0));

  SgLessThanOp * ifGuardExpression2 = buildLessThanOp (arrayExpression5,
      pointerDerefExpression3);

  SgIfStmt * minimumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression2, buildBasicBlock (assignmentStatement3));

  SgBasicBlock * minimumBasicBlock = buildBasicBlock ();

  appendStatement (minimumIfStatement, minimumBasicBlock);

  appendStatement (buildBreakStmt (), minimumBasicBlock);

  SgIntVal * minimumCaseExpression = buildIntVal (MINIMUM);

  SgCaseOptionStmt * minimumCaseStatement = buildCaseOptionStmt (
      minimumCaseExpression, minimumBasicBlock);

  /*
   * ======================================================
   * Switch statement on the reduction type with corresponding
   * cases
   * ======================================================
   */

  SgBasicBlock * switchBody = buildBasicBlock ();

  appendStatement (incrementCaseStatement, switchBody);

  appendStatement (minimumCaseStatement, switchBody);

  appendStatement (maximumCaseStatement, switchBody);

  SgSwitchStatement * switchStatement = buildSwitchStatement (
      variableDeclarations->getReference (reductionOperation), switchBody);

  /*
   * ======================================================
   * If statement determining whether this is thread 0
   * ======================================================
   */

  SgBasicBlock * outerIfBody = buildBasicBlock ();

  appendStatement (switchStatement, outerIfBody);

  SgExpression * ifGuardExpression3 = buildEqualityOp (
      variableDeclarations->getReference (threadID), buildIntVal (0));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression3, outerIfBody);

  appendStatement (ifStatement, subroutineScope);
}

void
CPPOpenCLReductionSubroutine::createSecondRoundOfReduceStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating second round of statements to perform reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * The increment reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      addExpression1);

  SgAddOp * addExpression2 = buildAddOp (arrayExpression1, arrayExpression2);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, addExpression2);

  SgIntVal * incrementCaseExpression = buildIntVal (INCREMENT);

  SgBasicBlock * incrementBasicBlock = buildBasicBlock ();

  appendStatement (assignmentStatement1, incrementBasicBlock);

  appendStatement (buildBreakStmt (), incrementBasicBlock);

  SgCaseOptionStmt * incrementCaseStatement = buildCaseOptionStmt (
      incrementCaseExpression, incrementBasicBlock);

  /*
   * ======================================================
   * The maximum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression3 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      addExpression3);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      arrayExpression3, arrayExpression4);

  SgGreaterThanOp * ifGuardExpression1 = buildGreaterThanOp (arrayExpression4,
      arrayExpression3);

  SgIfStmt * maximumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression1, buildBasicBlock (assignmentStatement2));

  SgBasicBlock * maximumBasicBlock = buildBasicBlock ();

  appendStatement (maximumIfStatement, maximumBasicBlock);

  appendStatement (buildBreakStmt (), maximumBasicBlock);

  SgIntVal * maximumCaseExpression = buildIntVal (MAXIMUM);

  SgCaseOptionStmt * maximumCaseStatement = buildCaseOptionStmt (
      maximumCaseExpression, maximumBasicBlock);

  /*
   * ======================================================
   * The minimum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression5 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression4 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression6 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      addExpression4);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      arrayExpression5, arrayExpression6);

  SgLessThanOp * ifGuardExpression2 = buildLessThanOp (arrayExpression6,
      arrayExpression5);

  SgIfStmt * minimumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression2, buildBasicBlock (assignmentStatement3));

  SgBasicBlock * minimumBasicBlock = buildBasicBlock ();

  appendStatement (minimumIfStatement, minimumBasicBlock);

  appendStatement (buildBreakStmt (), minimumBasicBlock);

  SgIntVal * minimumCaseExpression = buildIntVal (MINIMUM);

  SgCaseOptionStmt * minimumCaseStatement = buildCaseOptionStmt (
      minimumCaseExpression, minimumBasicBlock);

  /*
   * ======================================================
   * Switch statement on the reduction type with corresponding
   * cases
   * ======================================================
   */

  SgBasicBlock * switchBody = buildBasicBlock ();

  appendStatement (incrementCaseStatement, switchBody);

  appendStatement (minimumCaseStatement, switchBody);

  appendStatement (maximumCaseStatement, switchBody);

  SgSwitchStatement * switchStatement = buildSwitchStatement (
      variableDeclarations->getReference (reductionOperation), switchBody);

  /*
   * ======================================================
   * If statement
   * ======================================================
   */

  SgLessThanOp * ifGuardExpression3 = buildLessThanOp (
      variableDeclarations->getReference (threadID),
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgBasicBlock * ifBody = buildBasicBlock ();

  appendStatement (switchStatement, ifBody);

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression3, ifBody);

  /*
   * ======================================================
   * For loop statement
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (ifStatement, loopBody);

  SgGreaterThanOp * testExpression = buildGreaterThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgRshiftAssignOp * strideExpression = buildRshiftAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (1));

  SgForStatement * forStatement = buildForStatement (buildNullStatement (),
      buildExprStatement (testExpression), strideExpression, loopBody);

  /*
   * ======================================================
   * If statement
   * ======================================================
   */

  SgLessThanOp * ifGuardExpression4 = buildLessThanOp (
      variableDeclarations->getReference (threadID), buildOpaqueVarRefExp (
          OP2::Macros::warpSizeMacro, subroutineScope));

  SgBasicBlock * ifBody2 = buildBasicBlock ();

  appendStatement (forStatement, ifBody2);

  SgIfStmt * ifStatement2 =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression4, ifBody2);

  appendStatement (ifStatement2, subroutineScope);
}

void
CPPOpenCLReductionSubroutine::createFirstRoundOfReduceStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating first round of statements to perform reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * The increment reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), addExpression1);

  SgAddOp * addExpression2 = buildAddOp (arrayExpression1, arrayExpression2);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, addExpression2);

  SgIntVal * incrementCaseExpression = buildIntVal (INCREMENT);

  SgBasicBlock * incrementBasicBlock = buildBasicBlock ();

  appendStatement (assignmentStatement1, incrementBasicBlock);

  appendStatement (buildBreakStmt (), incrementBasicBlock);

  SgCaseOptionStmt * incrementCaseStatement = buildCaseOptionStmt (
      incrementCaseExpression, incrementBasicBlock);

  /*
   * ======================================================
   * The maximum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression3 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), addExpression3);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      arrayExpression3, arrayExpression4);

  SgGreaterThanOp * ifGuardExpression1 = buildGreaterThanOp (arrayExpression4,
      arrayExpression3);

  SgIfStmt * maximumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression1, buildBasicBlock (assignmentStatement2));

  SgBasicBlock * maximumBasicBlock = buildBasicBlock ();

  appendStatement (maximumIfStatement, maximumBasicBlock);

  appendStatement (buildBreakStmt (), maximumBasicBlock);

  SgIntVal * maximumCaseExpression = buildIntVal (MAXIMUM);

  SgCaseOptionStmt * maximumCaseStatement = buildCaseOptionStmt (
      maximumCaseExpression, maximumBasicBlock);

  /*
   * ======================================================
   * The minimum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression5 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression4 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression6 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), addExpression4);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      arrayExpression5, arrayExpression6);

  SgLessThanOp * ifGuardExpression2 = buildLessThanOp (arrayExpression6,
      arrayExpression5);

  SgIfStmt * minimumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression2, buildBasicBlock (assignmentStatement3));

  SgBasicBlock * minimumBasicBlock = buildBasicBlock ();

  appendStatement (minimumIfStatement, minimumBasicBlock);

  appendStatement (buildBreakStmt (), minimumBasicBlock);

  SgIntVal * minimumCaseExpression = buildIntVal (MINIMUM);

  SgCaseOptionStmt * minimumCaseStatement = buildCaseOptionStmt (
      minimumCaseExpression, minimumBasicBlock);

  /*
   * ======================================================
   * Switch statement on the reduction type with corresponding
   * cases
   * ======================================================
   */

  SgBasicBlock * switchBody = buildBasicBlock ();

  appendStatement (incrementCaseStatement, switchBody);

  appendStatement (minimumCaseStatement, switchBody);

  appendStatement (maximumCaseStatement, switchBody);

  SgSwitchStatement * switchStatement = buildSwitchStatement (
      variableDeclarations->getReference (reductionOperation), switchBody);

  /*
   * ======================================================
   * If statement
   * ======================================================
   */

  SgLessThanOp * ifGuardExpression3 = buildLessThanOp (
      variableDeclarations->getReference (threadID),
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgBasicBlock * ifBody = buildBasicBlock ();

  appendStatement (switchStatement, ifBody);

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression3, ifBody);

  /*
   * ======================================================
   * For loop statement
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (buildExprStatement (
      OpenCL::createWorkItemsSynchronisationCallStatement (subroutineScope)),
      loopBody);

  appendStatement (ifStatement, loopBody);

  SgGreaterThanOp * testExpression = buildGreaterThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildOpaqueVarRefExp (OP2::Macros::warpSizeMacro, subroutineScope));

  SgRshiftAssignOp * strideExpression = buildRshiftAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (1));

  SgForStatement * forStatement = buildForStatement (buildNullStatement (),
      buildExprStatement (testExpression), strideExpression, loopBody);

  appendStatement (forStatement, subroutineScope);
}

void
CPPOpenCLReductionSubroutine::createSharedVariableInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating shared variable initialisation statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (threadID));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression, variableDeclarations->getReference (reductionInput));

  appendStatement (assignmentStatement1, subroutineScope);
}

void
CPPOpenCLReductionSubroutine::createInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (threadID),
      OpenCL::getLocalWorkItemIDCallStatement (subroutineScope));

  appendStatement (assignmentStatement1, subroutineScope);

  SgRshiftOp * shiftExpression = buildRshiftOp (
      OpenCL::getLocalWorkGroupSizeCallStatement (subroutineScope),
      buildIntVal (1));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      shiftExpression);

  appendStatement (assignmentStatement2, subroutineScope);
}

void
CPPOpenCLReductionSubroutine::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createInitialisationStatements ();

  appendStatement (buildExprStatement (
      OpenCL::createWorkItemsSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  createSharedVariableInitialisationStatements ();

  createFirstRoundOfReduceStatements ();

  appendStatement (buildExprStatement (
      OpenCL::createWorkItemsSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (volatileSharedVariableName),
      variableDeclarations->getReference (sharedVariableName));

  appendStatement (assignmentStatement1, subroutineScope);

  createSecondRoundOfReduceStatements ();

  createThreadZeroReductionStatements ();
}

void
CPPOpenCLReductionSubroutine::createLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * We also need a shared memory pointer with the volatile
   * attribute to be compatible with Fermi
   * ======================================================
   */

  volatileSharedVariableName = getVolatileSharedMemoryDeclarationName (
      reduction->getBaseType (), reduction->getVariableSize ());

  SgVariableDeclaration * variableDeclaration1 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          volatileSharedVariableName, buildVolatileType (buildPointerType (
              reduction->getBaseType ())), subroutineScope);

  variableDeclaration1->get_declarationModifier ().get_storageModifier ().setOpenclLocal ();

  variableDeclarations->add (volatileSharedVariableName, variableDeclaration1);

  /*
   * ======================================================
   * Create thread ID and iteration counter variables
   * ======================================================
   */

  variableDeclarations->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));

  variableDeclarations->add (threadID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (threadID,
          buildIntType (), subroutineScope));
}

void
CPPOpenCLReductionSubroutine::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;

  /*
   * ======================================================
   * Declare the reduction result variable
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration1 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              reductionResult, buildVolatileType (buildPointerType (
                  reduction->getBaseType ())), subroutineScope,
              formalParameters);

  (*variableDeclaration1->get_variables ().begin ())->get_storageModifier ().setOpenclGlobal ();

  variableDeclarations->add (reductionResult, variableDeclaration1);

  /*
   * ======================================================
   * Declare the reduction input variable
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration2 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              reductionInput, reduction->getBaseType (), subroutineScope,
              formalParameters);

  (*variableDeclaration2->get_variables ().begin ())->get_storageModifier ().setOpenclPrivate ();

  variableDeclarations->add (reductionInput, variableDeclaration2);

  /*
   * ======================================================
   * Declare the reduction operation variable
   * ======================================================
   */

  SgVariableDeclaration
      * variableDeclaration3 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              reductionOperation, buildIntType (), subroutineScope,
              formalParameters);

  (*variableDeclaration3->get_variables ().begin ())->get_storageModifier ().setOpenclPrivate ();

  variableDeclarations->add (reductionOperation, variableDeclaration3);

  /*
   * ======================================================
   * Declare the shared memory variable
   * ======================================================
   */

  sharedVariableName = getSharedMemoryDeclarationName (
      reduction->getBaseType (), reduction->getVariableSize ());

  SgVariableDeclaration
      * variableDeclaration4 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              sharedVariableName, buildPointerType (reduction->getBaseType ()),
              subroutineScope, formalParameters);

  (*variableDeclaration4->get_variables ().begin ())->get_storageModifier ().setOpenclLocal ();

  variableDeclarations->add (sharedVariableName, variableDeclaration4);
}

CPPOpenCLReductionSubroutine::CPPOpenCLReductionSubroutine (
    SgScopeStatement * moduleScope, Reduction * reduction) :
  Subroutine <SgFunctionDeclaration> (reduction->getSubroutineName ()),
      reduction (reduction)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating reduction subroutine",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  subroutineHeaderStatement->get_functionModifier ().setOpenclKernel ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
