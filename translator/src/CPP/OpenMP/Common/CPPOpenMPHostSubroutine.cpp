


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


#include "CPPOpenMPHostSubroutine.h"
#include "CPPParallelLoop.h"
#include "CPPOpenMPKernelSubroutine.h"
#include "CPPModuleDeclarations.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "CompilerGeneratedNames.h"
#include "OpenMP.h"
#include "Exceptions.h"
#include "OP2.h"

SgBasicBlock *
CPPOpenMPHostSubroutine::createInitialiseNumberOfThreadsStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OpenMP;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise the number of threads",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (numberOfThreads),
      OpenMP::createGetMaximumNumberOfThreadsCallStatement (subroutineScope));

  appendStatement (assignmentStatement1, block);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (numberOfThreads), buildIntVal (1));

  appendStatement (assignmentStatement2, block);

  addTextForUnparser (assignmentStatement1, OpenMP::getIfDirectiveString (),
      AstUnparseAttribute::e_before);

  addTextForUnparser (assignmentStatement2, OpenMP::getElseDirectiveString (),
      AstUnparseAttribute::e_before);

  addTextForUnparser (assignmentStatement2, OpenMP::getEndIfDirectiveString (),
      AstUnparseAttribute::e_after);

  return block;
}

SgBasicBlock *
CPPOpenMPHostSubroutine::createOpDatTypeCastStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to type cast OP_DATs into their correct types",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpDatName (i)), buildOpaqueVarRefExp (data, subroutineScope));

    SgCastExp * castExpression = buildCastExp (dotExpression, buildPointerType (
        parallelLoop->getOpDatBaseType (i)));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        variableDeclarations->getReference (getOpDatLocalName (i)),
        castExpression);

    appendStatement (assignmentStatement, subroutineScope);
  }

  return block;
}

void
CPPOpenMPHostSubroutine::createOpDatTypeCastVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations to enable OP_DATs to be type cast into their correct types",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    std::string const & variableName = getOpDatLocalName (i);

    variableDeclarations->add (variableName,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName,
            buildPointerType (parallelLoop->getOpDatBaseType (i)),
            subroutineScope));
  }
}

void
CPPOpenMPHostSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace ReductionVariableNames;
  using namespace OpenMP;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isReductionRequired (i))
    {
      SgBasicBlock * loopBody = buildBasicBlock ();

      /*
       * ======================================================
       * New statement
       * ======================================================
       */

      SgMultiplyOp * arrayIndexExpression = buildMultiplyOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (64));

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getReductionArrayHostName (i)),
          arrayIndexExpression);

      SgPointerDerefExp * pointerDerefExpression = buildPointerDerefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)));

      SgPlusAssignOp * assignmentStatement = buildPlusAssignOp (
          pointerDerefExpression, arrayExpression);

      appendStatement (buildExprStatement (assignmentStatement), loopBody);

      /*
       * ======================================================
       * For loop statement
       * ======================================================
       */

      SgExprStatement * initialisationExpression = buildAssignStatement (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgLessThanOp * upperBoundExpression = buildLessThanOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), variableDeclarations->getReference (numberOfThreads));

      SgPlusPlusOp * strideExpression = buildPlusPlusOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgForStatement * forLoopStatement = buildForStatement (
          initialisationExpression, buildExprStatement (upperBoundExpression),
          strideExpression, loopBody);

      appendStatement (forLoopStatement, subroutineScope);
    }
  }
}

void
CPPOpenMPHostSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace ReductionVariableNames;
  using namespace OpenMP;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isReductionRequired (i))
    {
      SgBasicBlock * loopBody = buildBasicBlock ();

      /*
       * ======================================================
       * New statement
       * ======================================================
       */

      SgMultiplyOp * arrayIndexExpression = buildMultiplyOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (64));

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getReductionArrayHostName (i)),
          arrayIndexExpression);

      SgExpression * rhsExpression;

      if (isSgTypeInt (parallelLoop->getOpDatBaseType (i)))
      {
        rhsExpression = buildIntVal (0);
      }
      else if (isSgTypeFloat (parallelLoop->getOpDatBaseType (i)))
      {
        rhsExpression = buildFloatVal (0);
      }
      else if (isSgTypeDouble (parallelLoop->getOpDatBaseType (i)))
      {
        rhsExpression = buildDoubleVal (0);
      }
      else
      {
        throw Exceptions::ParallelLoop::UnsupportedBaseTypeException (
            "Reduction type not supported");
      }

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression, rhsExpression);

      appendStatement (assignmentStatement, loopBody);

      /*
       * ======================================================
       * For loop statement
       * ======================================================
       */

      SgExprStatement * initialisationExpression = buildAssignStatement (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgLessThanOp * upperBoundExpression = buildLessThanOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), variableDeclarations->getReference (numberOfThreads));

      SgPlusPlusOp * strideExpression = buildPlusPlusOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgForStatement * forLoopStatement = buildForStatement (
          initialisationExpression, buildExprStatement (upperBoundExpression),
          strideExpression, loopBody);

      appendStatement (forLoopStatement, subroutineScope);
    }
  }
}

void
CPPOpenMPHostSubroutine::createReductionDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for reductions",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (
      getIterationCounterVariableName (2),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2), buildIntType (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isReductionRequired (i))
    {
      std::string const & variableName = getReductionArrayHostName (i);

      SgMultiplyOp * mulitplyExpression = buildMultiplyOp (buildIntVal (64),
          buildIntVal (64));

      SgAddOp * addExpression =
          buildAddOp (buildIntVal (1), mulitplyExpression);

      variableDeclarations->add (variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildArrayType (parallelLoop->getOpDatBaseType (i),
                  addExpression), subroutineScope));
    }
  }
}

CPPOpenMPHostSubroutine::CPPOpenMPHostSubroutine (
    SgScopeStatement * moduleScope,
    CPPOpenMPKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations) :
  CPPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop),
      moduleDeclarations (moduleDeclarations)
{
  variableDeclarations->addVisibilityToSymbolsFromOuterScope (
      moduleDeclarations->getDeclarations ());
}
