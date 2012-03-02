


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


#include <CPPCUDAKernelSubroutine.h>
#include <CPPCUDAUserSubroutine.h>
#include <CPPReductionSubroutines.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CUDA.h>
#include <CompilerGeneratedNames.h>

void
CPPCUDAKernelSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isReductionRequired (i))
    {
      if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
      {
        SgBasicBlock * loopBody = buildBasicBlock ();

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatLocalName (dat_num)),
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

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

        SgAssignOp * initializationExpression = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (0));

        SgLessThanOp * upperBoundExpression = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgPlusPlusOp * strideExpression = buildPlusPlusOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        SgForStatement * forStatement = buildForStatement (buildExprStatement (
            initializationExpression),
            buildExprStatement (upperBoundExpression), strideExpression,
            loopBody);

        appendStatement (forStatement, subroutineScope);
      }
      else
      {
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
            variableDeclarations->getReference (getOpDatLocalName (dat_num)),
            rhsExpression);

        appendStatement (assignmentStatement, subroutineScope);
      }
    }
  }
}

void
CPPCUDAKernelSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isReductionRequired (i))
    {
      if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
      {
        SgBasicBlock * loopBody = buildBasicBlock ();

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatLocalName (dat_num)),
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (CUDA::getBlockId (
            BLOCK_X, subroutineScope), buildIntVal (
            parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), multiplyExpression);

        SgPntrArrRefExp * arrayExpression2 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                getReductionArrayDeviceName (dat_num)), addExpression);

        SgAddressOfOp * addressExpression = buildAddressOfOp (arrayExpression2);

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
         * Create reduction function call
         * ======================================================
         */

        SgExprListExp * actualParameters = buildExprListExp (addressExpression,
            arrayExpression1, reductionType);

        SgFunctionSymbol
            * reductionFunctionSymbol =
                isSgFunctionSymbol (
                    reductionSubroutines->getHeader (
                        parallelLoop->getReductionTuple (i))->get_symbol_from_symbol_table ());

        ROSE_ASSERT (reductionFunctionSymbol != NULL);

        SgFunctionCallExp * reductionFunctionCall = buildFunctionCallExp (
            reductionFunctionSymbol, actualParameters);

        appendStatement (buildExprStatement (reductionFunctionCall), loopBody);

        SgAssignOp * initializationExpression = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (0));

        SgLessThanOp * upperBoundExpression = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgPlusPlusOp * strideExpression = buildPlusPlusOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        SgForStatement * forStatement = buildForStatement (buildExprStatement (
            initializationExpression),
            buildExprStatement (upperBoundExpression), strideExpression,
            loopBody);

        appendStatement (forStatement, subroutineScope);
      }
      else
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (CUDA::getBlockId (
            BLOCK_X, subroutineScope), buildIntVal (
            parallelLoop->getOpDatDimension (i)));

        SgPntrArrRefExp * arrayExpression2 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                getReductionArrayDeviceName (dat_num)), multiplyExpression);

        SgAddressOfOp * addressExpression = buildAddressOfOp (arrayExpression2);

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
         * Create reduction function call
         * ======================================================
         */

        SgExprListExp * actualParameters = buildExprListExp (addressExpression,
            variableDeclarations->getReference (getOpDatLocalName (dat_num)),
            reductionType);

        SgFunctionSymbol
            * reductionFunctionSymbol =
                isSgFunctionSymbol (
                    reductionSubroutines->getHeader (
                        parallelLoop->getReductionTuple (i))->get_symbol_from_symbol_table ());

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
CPPCUDAKernelSubroutine::createReductionVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating declarations needed for reduction", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = getOpDatLocalName (dat_num);

      if (parallelLoop->isReductionRequired (i))
      {
        if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
        {
          variableDeclarations ->add (variableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, buildArrayType (parallelLoop->getOpDatBaseType (
                      i), buildIntVal (parallelLoop->getOpDatDimension (i))),
                  subroutineScope));
        }
        else
        {
          variableDeclarations ->add (variableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, parallelLoop->getOpDatBaseType (i),
                  subroutineScope));
        }
      }
    }
  }
}

CPPCUDAKernelSubroutine::CPPCUDAKernelSubroutine (
    SgScopeStatement * moduleScope, CPPCUDAUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  this->reductionSubroutines = reductionSubroutines;

  subroutineHeaderStatement->get_functionModifier ().setCudaKernel ();
}
