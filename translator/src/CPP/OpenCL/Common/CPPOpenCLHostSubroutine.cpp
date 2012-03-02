


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


#include "CPPOpenCLHostSubroutine.h"
#include "CPPOpenCLKernelSubroutine.h"
#include "CPPModuleDeclarations.h"
#include "CPPUserSubroutine.h"
#include "CPPOpenCLConstantDeclarations.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "OpenCL.h"
#include "CompilerGeneratedNames.h"
#include "OP2.h"
#include "Exceptions.h"

void
CPPOpenCLHostSubroutine::addOpDeclConstActualParameters (
    SgScopeStatement * scope, unsigned int argumentCounter)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Adding actual parameters for OP_DECL_CONSTs", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (vector <string>::const_iterator it =
      ((CPPUserSubroutine *) userSubroutine)->firstOpConstReference (); it
      != ((CPPUserSubroutine *) userSubroutine)->lastOpConstReference (); ++it)
  {
    SgFunctionCallExp * kernelArgumentExpression =
        OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
            variableDeclarations->getReference (OpenCL::kernelPointer),
            argumentCounter++, OpenCL::getMemoryType (scope),
            variableDeclarations->getReference (*it));

    SgBitOrOp * orExpression = buildBitOrOp (
        variableDeclarations->getReference (OpenCL::errorCode),
        kernelArgumentExpression);

    SgExprStatement * assignmentStatement = buildAssignStatement (
        variableDeclarations->getReference (OpenCL::errorCode), orExpression);

    appendStatement (assignmentStatement, scope);
  }
}

SgForStatement *
CPPOpenCLHostSubroutine::createReductionUpdateStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace ReductionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to update reduction variable",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * loopBody1 = buildBasicBlock ();

  if (parallelLoop->isArray (OP_DAT_ArgumentGroup) || parallelLoop->isPointer (
      OP_DAT_ArgumentGroup))
  {
    SgBasicBlock * loopBody2 = buildBasicBlock ();

    SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
        variableDeclarations->getReference (getReductionArrayHostName (
            OP_DAT_ArgumentGroup)), variableDeclarations->getReference (
            getIterationCounterVariableName (2)));

    SgMultiplyOp * multiplyExpression =
        buildMultiplyOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)), buildIntVal (
            parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

    SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
        getIterationCounterVariableName (2)), multiplyExpression);

    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpDatName (OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (
        OP2::RunTimeVariableNames::data, subroutineScope));

    SgCastExp * castExpression = buildCastExp (dotExpression, buildPointerType (
        parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup)));

    SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (castExpression,
        addExpression);

    if (parallelLoop->isIncremented (OP_DAT_ArgumentGroup))
    {
      SgPlusAssignOp * plusPlusExpression = buildPlusAssignOp (
          arrayExpression1, arrayExpression2);

      appendStatement (buildExprStatement (plusPlusExpression), loopBody2);
    }
    else if (parallelLoop->isMaximised (OP_DAT_ArgumentGroup))
    {
      SgFunctionCallExp * maxCallExpression =
          OP2::Macros::createMaxCallStatement (subroutineScope,
              arrayExpression1, arrayExpression2);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression1, maxCallExpression);

      appendStatement (assignmentStatement, loopBody2);
    }
    else if (parallelLoop->isMinimised (OP_DAT_ArgumentGroup))
    {
      SgFunctionCallExp * minCallExpression =
          OP2::Macros::createMinCallStatement (subroutineScope,
              arrayExpression1, arrayExpression2);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression1, minCallExpression);

      appendStatement (assignmentStatement, loopBody2);
    }

    SgAssignOp * initialisationExpression2 =
        buildAssignOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)), buildIntVal (0));

    SgLessThanOp * upperBoundExpression2 =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)), buildIntVal (
            parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

    SgPlusPlusOp * strideExpression2 =
        buildPlusPlusOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)));

    SgForStatement * forStatement2 = buildForStatement (buildExprStatement (
        initialisationExpression2), buildExprStatement (upperBoundExpression2),
        strideExpression2, loopBody2);

    appendStatement (forStatement2, loopBody1);
  }
  else
  {
    SgPointerDerefExp * pointerDerefenceExpression1 = buildPointerDerefExp (
        variableDeclarations->getReference (getReductionArrayHostName (
            OP_DAT_ArgumentGroup)));

    SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (getReductionArrayHostName (
            OP_DAT_ArgumentGroup)), variableDeclarations->getReference (
            getIterationCounterVariableName (1)));

    if (parallelLoop->isIncremented (OP_DAT_ArgumentGroup))
    {
      SgPlusAssignOp * plusPlusExpression = buildPlusAssignOp (
          pointerDerefenceExpression1, arrayExpression);

      appendStatement (buildExprStatement (plusPlusExpression), loopBody1);
    }
    else if (parallelLoop->isMaximised (OP_DAT_ArgumentGroup))
    {
      SgPointerDerefExp * pointerDerefenceExpression2 = buildPointerDerefExp (
          variableDeclarations->getReference (getReductionArrayHostName (
              OP_DAT_ArgumentGroup)));

      SgFunctionCallExp * maxCallExpression =
          OP2::Macros::createMaxCallStatement (subroutineScope,
              pointerDerefenceExpression2, arrayExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          pointerDerefenceExpression1, maxCallExpression);

      appendStatement (assignmentStatement, loopBody1);
    }
    else if (parallelLoop->isMinimised (OP_DAT_ArgumentGroup))
    {
      SgPointerDerefExp * pointerDerefenceExpression2 = buildPointerDerefExp (
          variableDeclarations->getReference (getReductionArrayHostName (
              OP_DAT_ArgumentGroup)));

      SgFunctionCallExp * minCallExpression =
          OP2::Macros::createMinCallStatement (subroutineScope,
              pointerDerefenceExpression2, arrayExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          pointerDerefenceExpression1, minCallExpression);

      appendStatement (assignmentStatement, loopBody1);
    }
  }

  SgAssignOp * initialisationExpression1 = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression1 = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      variableDeclarations->getReference (OpenCL::blocksPerGrid));

  SgPlusPlusOp * strideExpression1 = buildPlusPlusOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgForStatement * forStatement1 = buildForStatement (buildExprStatement (
      initialisationExpression1), buildExprStatement (upperBoundExpression1),
      strideExpression1, loopBody1);

  return forStatement1;
}

void
CPPOpenCLHostSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgFunctionCallExp
      * moveReductionArraysToHostCall =
          OpenCL::OP2RuntimeSupport::getMoveReductionArraysFromDeviceToHostCallStatement (
              subroutineScope, variableDeclarations->getReference (
                  reductionBytes));

  appendStatement (buildExprStatement (moveReductionArraysToHostCall),
      subroutineScope);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isReductionRequired (i))
    {
      appendStatement (createReductionUpdateStatements (i), subroutineScope);
    }
  }
}

SgForStatement *
CPPOpenCLHostSubroutine::createReductionInitialisationStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace OP2::RunTimeVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise reduction variable",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * For loop statement to initialise reduction array
   * ======================================================
   */

  SgBasicBlock * loopBody1 = buildBasicBlock ();

  SgExpression * rhsExpression;

  if (parallelLoop->isIncremented (OP_DAT_ArgumentGroup))
  {
    if (isSgTypeInt (parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup)))
    {
      rhsExpression = buildIntVal (0);
    }
    else if (isSgTypeFloat (parallelLoop->getOpDatBaseType (
        OP_DAT_ArgumentGroup)))
    {
      rhsExpression = buildFloatVal (0);
    }
    else if (isSgTypeDouble (parallelLoop->getOpDatBaseType (
        OP_DAT_ArgumentGroup)))
    {
      rhsExpression = buildDoubleVal (0);
    }
    else
    {
      throw Exceptions::ParallelLoop::UnsupportedBaseTypeException (
          "Reduction type not supported");
    }
  }
  else
  {
    rhsExpression = variableDeclarations->getReference (getOpDatName (
        OP_DAT_ArgumentGroup));
  }

  if (parallelLoop->isArray (OP_DAT_ArgumentGroup) || parallelLoop->isPointer (
      OP_DAT_ArgumentGroup))
  {
    SgBasicBlock * loopBody2 = buildBasicBlock ();

    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpDatName (OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (data,
        subroutineScope));

    SgCastExp * castExpression = buildCastExp (dotExpression, buildPointerType (
        parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup)));

    SgMultiplyOp * multiplyExpression =
        buildMultiplyOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)), buildIntVal (
            parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

    SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
        getIterationCounterVariableName (2)), multiplyExpression);

    SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (castExpression,
        addExpression);

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayExpression, rhsExpression);

    appendStatement (assignmentStatement, loopBody2);

    SgAssignOp * initialisationExpression2 =
        buildAssignOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)), buildIntVal (0));

    SgLessThanOp * upperBoundExpression2 =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)), buildIntVal (
            parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

    SgPlusPlusOp * strideExpression2 =
        buildPlusPlusOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)));

    SgForStatement * forStatement2 = buildForStatement (buildExprStatement (
        initialisationExpression2), buildExprStatement (upperBoundExpression2),
        strideExpression2, loopBody2);

    appendStatement (forStatement2, loopBody1);
  }
  else
  {
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpDatName (OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (data,
        subroutineScope));

    SgCastExp * castExpression = buildCastExp (dotExpression, buildPointerType (
        parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup)));

    SgPntrArrRefExp * arrayExpression =
        buildPntrArrRefExp (castExpression, variableDeclarations->getReference (
            getIterationCounterVariableName (1)));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayExpression, rhsExpression);

    appendStatement (assignmentStatement, loopBody1);
  }

  SgAssignOp * initialisationExpression1 = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression1 = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      variableDeclarations->getReference (OpenCL::blocksPerGrid));

  SgPlusPlusOp * strideExpression1 = buildPlusPlusOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgForStatement * forStatement1 = buildForStatement (buildExprStatement (
      initialisationExpression1), buildExprStatement (upperBoundExpression1),
      strideExpression1, loopBody1);

  return forStatement1;
}

void
CPPOpenCLHostSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace ReductionVariableNames;
  using namespace OP2::RunTimeVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (reductionBytes), buildIntVal (0));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (reductionSharedMemorySize),
      buildIntVal (0));

  appendStatement (assignmentStatement2, subroutineScope);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isReductionRequired (i))
    {
      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (data, subroutineScope));

      if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
      {
        SgCastExp * castExpression = buildCastExp (dotExpression,
            buildPointerType (parallelLoop->getOpDatBaseType (i)));

        SgExprStatement * assignmentStatement3 = buildAssignStatement (
            variableDeclarations->getReference (getReductionArrayHostName (i)),
            castExpression);

        appendStatement (assignmentStatement3, subroutineScope);
      }
      else
      {
        SgCastExp * castExpression = buildCastExp (dotExpression,
            buildPointerType (parallelLoop->getOpDatBaseType (i)));

        SgAddressOfOp * addressOfExpression = buildAddressOfOp (castExpression);

        SgExprStatement * assignmentStatement3 = buildAssignStatement (
            variableDeclarations->getReference (getReductionArrayHostName (i)),
            addressOfExpression);

        appendStatement (assignmentStatement3, subroutineScope);
      }

      SgSizeOfOp * sizeOfExpression = buildSizeOfOp (
          parallelLoop->getOpDatBaseType (i));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          variableDeclarations->getReference (OpenCL::blocksPerGrid),
          sizeOfExpression);

      SgFunctionCallExp * roundUpCallExpression;

      if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
      {
        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
            multiplyExpression1, buildIntVal (parallelLoop->getOpDatDimension (
                i)));

        roundUpCallExpression = OP2::Macros::createRoundUpCallStatement (
            subroutineScope, multiplyExpression2);
      }
      else
      {
        roundUpCallExpression = OP2::Macros::createRoundUpCallStatement (
            subroutineScope, multiplyExpression1);
      }

      SgPlusAssignOp * assignmentStatement3 = buildPlusAssignOp (
          variableDeclarations->getReference (reductionBytes),
          roundUpCallExpression);

      appendStatement (buildExprStatement (assignmentStatement3),
          subroutineScope);

      SgFunctionCallExp * maxCallExpression =
          OP2::Macros::createMaxCallStatement (subroutineScope,
              variableDeclarations->getReference (reductionSharedMemorySize),
              sizeOfExpression);

      SgExprStatement * assignmentStatement4 = buildAssignStatement (
          variableDeclarations->getReference (reductionSharedMemorySize),
          maxCallExpression);

      appendStatement (assignmentStatement4, subroutineScope);
    }
  }

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgFunctionCallExp * reallocateReductionArraysExpression =
      OpenCL::OP2RuntimeSupport::getReallocateReductionArraysCallStatement (
          subroutineScope, variableDeclarations->getReference (reductionBytes));

  appendStatement (buildExprStatement (reallocateReductionArraysExpression),
      subroutineScope);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      variableDeclarations->getReference (reductionBytes), buildIntVal (0));

  appendStatement (assignmentStatement5, subroutineScope);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isReductionRequired (i))
    {
      SgAddOp
          * addExpression1 =
              buildAddOp (
                  OpenCL::OP2RuntimeSupport::getPointerToMemoryAllocatedForHostReductionArray (
                      subroutineScope), variableDeclarations->getReference (
                      reductionBytes));

      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (data, subroutineScope));

      SgExprStatement * assignmentStatement4 = buildAssignStatement (
          dotExpression1, addExpression1);

      appendStatement (assignmentStatement4, subroutineScope);

      SgAddOp
          * addExpression2 =
              buildAddOp (
                  OpenCL::OP2RuntimeSupport::getPointerToMemoryAllocatedForDeviceReductionArray (
                      subroutineScope), variableDeclarations->getReference (
                      reductionBytes));

      SgDotExp * dotExpression2 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (data_d, subroutineScope));

      SgExprStatement * assignmentStatement5 = buildAssignStatement (
          dotExpression2, addExpression2);

      appendStatement (assignmentStatement5, subroutineScope);

      appendStatement (createReductionInitialisationStatements (i),
          subroutineScope);

      /*
       * ======================================================
       * New statement
       * ======================================================
       */

      SgSizeOfOp * sizeOfExpression = buildSizeOfOp (
          parallelLoop->getOpDatBaseType (i));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          variableDeclarations->getReference (OpenCL::blocksPerGrid),
          sizeOfExpression);

      SgFunctionCallExp * roundUpCallExpression;

      if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
      {
        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
            multiplyExpression1, buildIntVal (parallelLoop->getOpDatDimension (
                i)));

        roundUpCallExpression = OP2::Macros::createRoundUpCallStatement (
            subroutineScope, multiplyExpression2);
      }
      else
      {
        roundUpCallExpression = OP2::Macros::createRoundUpCallStatement (
            subroutineScope, multiplyExpression1);
      }

      SgPlusAssignOp * assignmentStatement6 = buildPlusAssignOp (
          variableDeclarations->getReference (reductionBytes),
          roundUpCallExpression);

      appendStatement (buildExprStatement (assignmentStatement6),
          subroutineScope);
    }
  }

  SgFunctionCallExp
      * moveReductionArraysToDeviceCall =
          OpenCL::OP2RuntimeSupport::getMoveReductionArraysFromHostToDeviceCallStatement (
              subroutineScope, variableDeclarations->getReference (
                  reductionBytes));

  appendStatement (buildExprStatement (moveReductionArraysToDeviceCall),
      subroutineScope);
}

void
CPPOpenCLHostSubroutine::createReductionDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace ReductionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));

  variableDeclarations->add (
      getIterationCounterVariableName (2),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2), buildIntType (), subroutineScope));

  variableDeclarations->add (reductionBytes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          reductionBytes, buildIntType (), subroutineScope));

  variableDeclarations->add (reductionSharedMemorySize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          reductionSharedMemorySize, buildIntType (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isReductionRequired (i))
    {
      string const reductionArrayHostName = getReductionArrayHostName (i);

      Debug::getInstance ()->debugMessage ("Creating host reduction pointer '"
          + reductionArrayHostName + "'", Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgVariableDeclaration * reductionArrayHost =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionArrayHostName, buildPointerType (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope);

      variableDeclarations->add (reductionArrayHostName, reductionArrayHost);
    }
  }
}

void
CPPOpenCLHostSubroutine::createKernelCallEpilogueStatements (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating statements to launch OpenCL kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Assert statement
   * ======================================================
   */

  SgEqualityOp * equalityExpression1 = buildEqualityOp (
      variableDeclarations->getReference (OpenCL::errorCode),
      buildOpaqueVarRefExp (OpenCL::CL_SUCCESS, scope));

  appendStatement (buildExprStatement (
      OpenCL::OP2RuntimeSupport::getAssertMessage (subroutineScope,
          equalityExpression1, buildStringVal (
              "Error setting OpenCL kernel arguments"))), scope);

  /*
   * ======================================================
   * Execute kernel statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode),
      OpenCL::getEnqueueKernelCallExpression (subroutineScope,
          buildOpaqueVarRefExp (OpenCL::commandQueue, scope),
          variableDeclarations->getReference (OpenCL::kernelPointer),
          variableDeclarations->getReference (OpenCL::totalThreadNumber),
          variableDeclarations->getReference (OpenCL::threadsPerBlock),
          variableDeclarations->getReference (OpenCL::event)));

  appendStatement (assignmentStatement1, scope);

  /*
   * ======================================================
   * Assert statement
   * ======================================================
   */

  SgEqualityOp * equalityExpression2 = buildEqualityOp (
      variableDeclarations->getReference (OpenCL::errorCode),
      buildOpaqueVarRefExp (OpenCL::CL_SUCCESS, scope));

  appendStatement (
      buildExprStatement (OpenCL::OP2RuntimeSupport::getAssertMessage (
          subroutineScope, equalityExpression2, buildStringVal (
              "Error executing OpenCL kernel"))), scope);

  /*
   * ======================================================
   * Complete device commands statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode),
      OpenCL::getFinishCommandQueueCallExpression (subroutineScope,
          buildOpaqueVarRefExp (OpenCL::commandQueue, scope)));

  appendStatement (assignmentStatement2, scope);

  /*
   * ======================================================
   * Assert statement
   * ======================================================
   */

  SgEqualityOp * equalityExpression3 = buildEqualityOp (
      variableDeclarations->getReference (OpenCL::errorCode),
      buildOpaqueVarRefExp (OpenCL::CL_SUCCESS, scope));

  appendStatement (buildExprStatement (
      OpenCL::OP2RuntimeSupport::getAssertMessage (subroutineScope,
          equalityExpression3, buildStringVal (
              "Error completing device command queue"))), scope);
}

void
CPPOpenCLHostSubroutine::createOpenCLConfigurationLaunchDeclarations ()
{
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage (
      "Creating OpenCL configuration launch local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (OpenCL::blocksPerGrid,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::blocksPerGrid, OpenCL::getSizeType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::threadsPerBlock,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::threadsPerBlock, OpenCL::getSizeType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::totalThreadNumber,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::totalThreadNumber, OpenCL::getSizeType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::sharedMemorySize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::sharedMemorySize, OpenCL::getSizeType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::errorCode,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::errorCode, OpenCL::getSignedIntegerType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::event,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::event, OpenCL::getEventType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::kernelPointer,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::kernelPointer, OpenCL::getKernelType (subroutineScope),
          subroutineScope));
}

CPPOpenCLHostSubroutine::CPPOpenCLHostSubroutine (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations,
    CPPUserSubroutine * userSubroutine,
    CPPOpenCLConstantDeclarations * constantDeclarations) :
  CPPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop),
      moduleDeclarations (moduleDeclarations), userSubroutine (userSubroutine),
      constantDeclarations (constantDeclarations)
{
  variableDeclarations->addVisibilityToSymbolsFromOuterScope (
      moduleDeclarations->getDeclarations ());

  variableDeclarations->addVisibilityToSymbolsFromOuterScope (
      constantDeclarations->getDeclarations ());
}
