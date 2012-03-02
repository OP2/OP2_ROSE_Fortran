


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


#include "CPPOpenMPHostSubroutineIndirectLoop.h"
#include "CPPUserSubroutine.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "RoseHelper.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "OP2.h"
#include "OpenMP.h"

void
CPPOpenMPHostSubroutineIndirectLoop::createKernelFunctionCallStatement (
    SgScopeStatement * scope)
{
  using namespace SageInterface;
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statement to call OpenMP kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i))
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (64));

        SgAddOp * addExpression = buildAddOp (
            variableDeclarations->getReference (getReductionArrayHostName (i)),
            multiplyExpression);

        actualParameters->append_expression (addExpression);
      }
      else
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (getOpDatLocalName (i)));
      }
    }
  }

  unsigned int arrayIndex = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
            ind_maps, buildArrayType (buildIntType ()), NULL, subroutineScope);

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclaration), buildIntVal (arrayIndex));

        SgArrowExp * arrowExpression = buildArrowExp (
            variableDeclarations->getReference (planRet), arrayExpression);

        actualParameters->append_expression (arrowExpression);

        arrayIndex++;
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isIndirect (i))
    {
      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          loc_maps, buildArrayType (buildIntType ()), NULL, subroutineScope);

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclaration), buildIntVal (i - 1));

      SgArrowExp * arrowExpression = buildArrowExp (
          variableDeclarations->getReference (planRet), arrayExpression);

      actualParameters->append_expression (arrowExpression);
    }
  }

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          ind_sizes, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          ind_offs, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          blkmap, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          offset, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          nelems, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          thrcol, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          nthrcol, subroutineScope)));

  actualParameters->append_expression (variableDeclarations->getReference (
      blockOffset));

  actualParameters->append_expression (variableDeclarations->getReference (
      blockID));

  SgFunctionCallExp * functionCallExpression = buildFunctionCallExp (
      calleeSubroutine->getSubroutineName (), buildVoidType (),
      actualParameters, subroutineScope);

  appendStatement (buildExprStatement (functionCallExpression), scope);
}

void
CPPOpenMPHostSubroutineIndirectLoop::createOpenMPLoopStatements (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace OpenMP;

  Debug::getInstance ()->debugMessage ("Creating OpenMP for loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock ();

  createKernelFunctionCallStatement (loopBody);

  /*
   * ======================================================
   * For loop statement
   * ======================================================
   */

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (blockID), buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (blockID),
      variableDeclarations->getReference (nblocks));

  SgPlusPlusOp * strideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (blockID));

  SgForStatement * forLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (forLoopStatement, scope);

  /*
   * ======================================================
   * The OpenMP directive
   * ======================================================
   */

  std::vector <SgVarRefExp *> privateVariableReferences;

  privateVariableReferences.push_back (variableDeclarations->getReference (
      blockID));

  addTextForUnparser (forLoopStatement,
      OpenMP::getParallelLoopDirectiveString () + OpenMP::getPrivateClause (
          privateVariableReferences), AstUnparseAttribute::e_before);
}

SgBasicBlock *
CPPOpenMPHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace OpenMP;

  Debug::getInstance ()->debugMessage (
      "Creating plan function execution loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (blockOffset), buildIntVal (0));

  appendStatement (assignmentStatement1, block);

  /*
   * ======================================================
   * For loop body
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (ncolblk, subroutineScope),
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgArrowExp * arrowExpression2 = buildArrowExp (
      variableDeclarations->getReference (planRet), arrayExpression2);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (nblocks), arrowExpression2);

  appendStatement (assignmentStatement2, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  createOpenMPLoopStatements (loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement5 = buildExprStatement (
      buildPlusAssignOp (variableDeclarations->getReference (blockOffset),
          variableDeclarations->getReference (nblocks)));

  appendStatement (assignmentStatement5, loopBody);

  /*
   * ======================================================
   * For loop statement
   * ======================================================
   */

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgArrowExp * arrowExpression = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          ncolors, subroutineScope));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      arrowExpression);

  SgPlusPlusOp * strideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgForStatement * forLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (forLoopStatement, block);

  return block;
}

SgExprStatement *
CPPOpenMPHostSubroutineIndirectLoop::createPlanFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage ("Creating plan function call statement",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (variableDeclarations->getReference (
      getUserSubroutineName ()));

  actualParameters->append_expression (variableDeclarations->getReference (
      getOpSetName ()));

  actualParameters->append_expression (variableDeclarations->getReference (
      getPartitionSizeVariableName (parallelLoop->getUserSubroutineName ())));

  actualParameters->append_expression (buildIntVal (
      parallelLoop->getNumberOfOpDatArgumentGroups ()));

  actualParameters->append_expression (variableDeclarations->getReference (
      opDatArray));

  actualParameters->append_expression (buildIntVal (
      parallelLoop->getNumberOfDistinctIndirectOpDats ()));

  actualParameters->append_expression (variableDeclarations->getReference (
      indirectionDescriptorArray));

  SgFunctionCallExp * functionCall = buildFunctionCallExp (OP2::OP_PLAN_GET,
      buildVoidType (), actualParameters, subroutineScope);

  SgExprStatement * assignmentStatement = buildAssignStatement (
      variableDeclarations->getReference (planRet), functionCall);

  return assignmentStatement;
}

void
CPPOpenMPHostSubroutineIndirectLoop::createStatements ()
{
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  appendStatementList (
      createInitialiseNumberOfThreadsStatements ()->getStatementList (),
      subroutineScope);

  appendStatementList (
      createInitialisePlanFunctionArrayStatements ()->getStatementList (),
      subroutineScope);

  appendStatementList (createOpDatTypeCastStatements ()->getStatementList (),
      subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  appendStatement (createPlanFunctionCallStatement (), subroutineScope);

  appendStatementList (
      createPlanFunctionExecutionStatements ()->getStatementList (),
      subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPOpenMPHostSubroutineIndirectLoop::createOpenMPLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating OpenMP local variable declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));

  variableDeclarations->add (OpenMP::numberOfThreads,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::numberOfThreads, buildIntType (), subroutineScope));
}

void
CPPOpenMPHostSubroutineIndirectLoop::createPlanFunctionDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations for plan function",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (blockID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (blockID,
          buildIntType (), subroutineScope));

  variableDeclarations->add (blockOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          blockOffset, buildIntType (), subroutineScope));

  variableDeclarations->add (nblocks,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nblocks,
          buildIntType (), subroutineScope));

  variableDeclarations->add (opDatArray,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          opDatArray, buildArrayType (buildOpaqueType (OP2::OP_ARG,
              subroutineScope), buildIntVal (
              parallelLoop->getNumberOfOpDatArgumentGroups ())),
          subroutineScope));

  variableDeclarations->add (indirectionDescriptorArray,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          indirectionDescriptorArray, buildArrayType (buildIntType (),
              buildIntVal (parallelLoop->getNumberOfOpDatArgumentGroups ())),
          subroutineScope));

  variableDeclarations->add (planRet,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (planRet,
          buildPointerType (buildOpaqueType (OP2::OP_PLAN, subroutineScope)),
          subroutineScope));
}

void
CPPOpenMPHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createOpenMPLocalVariableDeclarations ();

  createOpDatTypeCastVariableDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }

  createPlanFunctionDeclarations ();
}

CPPOpenMPHostSubroutineIndirectLoop::CPPOpenMPHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    CPPOpenMPKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations) :
  CPPOpenMPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop,
      moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
