


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


#include "CPPOpenCLHostSubroutineIndirectLoop.h"
#include "CPPParallelLoop.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "CompilerGeneratedNames.h"
#include "RoseHelper.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "OpenCL.h"
#include "OP2.h"

void
CPPOpenCLHostSubroutineIndirectLoop::createKernelFunctionCallStatement (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to set up OpenCL kernel arguments",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Assign the kernel pointer using the run-time support
   * for OpenCL
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::kernelPointer),
      OpenCL::OP2RuntimeSupport::getKernel (subroutineScope,
          calleeSubroutine->getSubroutineName ()));

  appendStatement (assignmentStatement1, scope);

  /*
   * ======================================================
   * OpenCL kernel arguments for OP_DATs
   * ======================================================
   */

  unsigned int argumentCounter = 0;
  bool firstAssignment = true;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (data_d, scope));

      SgFunctionCallExp * kernelArgumentExpression =
          OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
              variableDeclarations->getReference (OpenCL::kernelPointer),
              argumentCounter++, OpenCL::getMemoryType (scope), dotExpression);

      if (firstAssignment)
      {
        firstAssignment = false;

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (OpenCL::errorCode),
            kernelArgumentExpression);

        appendStatement (assignmentStatement, scope);
      }
      else
      {
        SgBitOrOp * orExpression = buildBitOrOp (
            variableDeclarations->getReference (OpenCL::errorCode),
            kernelArgumentExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (OpenCL::errorCode),
            orExpression);

        appendStatement (assignmentStatement, scope);
      }
    }
  }

  /*
   * ======================================================
   * OpenCL kernel arguments for global-to-local memory
   * mappings of indirect OP_DATs
   * ======================================================
   */

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

        SgFunctionCallExp * kernelArgumentExpression =
            OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
                variableDeclarations->getReference (OpenCL::kernelPointer),
                argumentCounter++, OpenCL::getMemoryType (scope),
                arrowExpression);

        SgBitOrOp * orExpression = buildBitOrOp (
            variableDeclarations->getReference (OpenCL::errorCode),
            kernelArgumentExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (OpenCL::errorCode),
            orExpression);

        appendStatement (assignmentStatement, scope);

        arrayIndex++;
      }
    }
  }

  /*
   * ======================================================
   * OpenCL kernel arguments for local-to-global memory
   * mappings of OP_DATs
   * ======================================================
   */

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

      SgFunctionCallExp
          * kernelArgumentExpression =
              OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
                  variableDeclarations->getReference (OpenCL::kernelPointer),
                  argumentCounter++, OpenCL::getMemoryType (scope),
                  arrowExpression);

      SgBitOrOp * orExpression = buildBitOrOp (
          variableDeclarations->getReference (OpenCL::errorCode),
          kernelArgumentExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          variableDeclarations->getReference (OpenCL::errorCode), orExpression);

      appendStatement (assignmentStatement, scope);
    }
  }

  /*
   * ======================================================
   * OpenCL kernel arguments for plan function variable
   * ======================================================
   */

  SgArrowExp * arrowExpression2 = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          ind_sizes, scope));

  SgFunctionCallExp * kernelArgumentExpression2 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter++, OpenCL::getMemoryType (scope), arrowExpression2);

  SgBitOrOp * orExpression2 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression2);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression2);

  appendStatement (assignmentStatement2, scope);

  /*
   * ======================================================
   * OpenCL kernel arguments for plan function variable
   * ======================================================
   */

  SgArrowExp * arrowExpression3 = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          ind_offs, scope));

  SgFunctionCallExp * kernelArgumentExpression3 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter++, OpenCL::getMemoryType (scope), arrowExpression3);

  SgBitOrOp * orExpression3 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression3);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression3);

  appendStatement (assignmentStatement3, scope);

  /*
   * ======================================================
   * OpenCL kernel arguments for plan function variable
   * ======================================================
   */

  SgArrowExp * arrowExpression4 = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          blkmap, scope));

  SgFunctionCallExp * kernelArgumentExpression4 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter++, OpenCL::getMemoryType (scope), arrowExpression4);

  SgBitOrOp * orExpression4 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression4);

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression4);

  appendStatement (assignmentStatement4, scope);

  /*
   * ======================================================
   * OpenCL kernel arguments for plan function variable
   * ======================================================
   */

  SgArrowExp * arrowExpression5 = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          offset, scope));

  SgFunctionCallExp * kernelArgumentExpression5 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter++, OpenCL::getMemoryType (scope), arrowExpression5);

  SgBitOrOp * orExpression5 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression5);

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression5);

  appendStatement (assignmentStatement5, scope);

  /*
   * ======================================================
   * OpenCL kernel arguments for plan function variable
   * ======================================================
   */

  SgArrowExp * arrowExpression6 = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          nelems, scope));

  SgFunctionCallExp * kernelArgumentExpression6 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter++, OpenCL::getMemoryType (scope), arrowExpression6);

  SgBitOrOp * orExpression6 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression6);

  SgExprStatement * assignmentStatement6 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression6);

  appendStatement (assignmentStatement6, scope);

  /*
   * ======================================================
   * OpenCL kernel arguments for plan function variable
   * ======================================================
   */

  SgArrowExp * arrowExpression7 = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          nthrcol, scope));

  SgFunctionCallExp * kernelArgumentExpression7 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter++, OpenCL::getMemoryType (scope), arrowExpression7);

  SgBitOrOp * orExpression7 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression7);

  SgExprStatement * assignmentStatement7 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression7);

  appendStatement (assignmentStatement7, scope);

  /*
   * ======================================================
   * OpenCL kernel arguments for plan function variable
   * ======================================================
   */

  SgArrowExp * arrowExpression8 = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          thrcol, scope));

  SgFunctionCallExp * kernelArgumentExpression8 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter++, OpenCL::getMemoryType (scope), arrowExpression8);

  SgBitOrOp * orExpression8 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression8);

  SgExprStatement * assignmentStatement8 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression8);

  appendStatement (assignmentStatement8, scope);

  /*
   * ======================================================
   * OpenCL kernel arguments for plan function variable
   * ======================================================
   */

  SgFunctionCallExp * kernelArgumentExpression9 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter++, OpenCL::getMemoryType (scope),
          variableDeclarations->getReference (blockOffset));

  SgBitOrOp * orExpression9 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression9);

  SgExprStatement * assignmentStatement9 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression9);

  appendStatement (assignmentStatement9, scope);

  /*
   * ======================================================
   * OpenCL kernel arguments for dynamic shared memory size
   * ======================================================
   */

  SgFunctionCallExp * kernelArgumentExpression10 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter++, OpenCL::getMemoryType (scope),
          variableDeclarations->getReference (OpenCL::sharedMemorySize));

  SgBitOrOp * orExpression10 = buildBitOrOp (
      variableDeclarations->getReference (OpenCL::errorCode),
      kernelArgumentExpression10);

  SgExprStatement * assignmentStatement10 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression10);

  appendStatement (assignmentStatement10, scope);

  /*
   * ======================================================
   * OP_DECL_CONST actual parameters
   * ======================================================
   */

  addOpDeclConstActualParameters (scope, argumentCounter);
}

SgBasicBlock *
CPPOpenCLHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating plan function execution statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

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
      variableDeclarations->getReference (getIterationCounterVariableName (3)));

  SgArrowExp * arrowExpression2 = buildArrowExp (
      variableDeclarations->getReference (planRet), arrayExpression2);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::blocksPerGrid),
      arrowExpression2);

  appendStatement (assignmentStatement2, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgArrowExp * arrowExpression3 = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          nshared, subroutineScope));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      arrowExpression3);

  appendStatement (assignmentStatement3, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::threadsPerBlock),
      variableDeclarations->getReference (getBlockSizeVariableName (
          parallelLoop->getUserSubroutineName ())));

  appendStatement (assignmentStatement4, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgMultiplyOp * multiplyExpression5 = buildMultiplyOp (
      variableDeclarations->getReference (OpenCL::threadsPerBlock),
      variableDeclarations->getReference (OpenCL::blocksPerGrid));

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::totalThreadNumber),
      multiplyExpression5);

  appendStatement (assignmentStatement5, subroutineScope);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  createKernelFunctionCallStatement (loopBody);

  createKernelCallEpilogueStatements (loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement6 = buildExprStatement (
      buildPlusAssignOp (variableDeclarations->getReference (blockOffset),
          variableDeclarations->getReference (OpenCL::blocksPerGrid)));

  appendStatement (assignmentStatement6, loopBody);

  /*
   * ======================================================
   * For loop header
   * ======================================================
   */

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (3)),
      buildIntVal (0));

  SgArrowExp * arrowExpression = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          ncolors, subroutineScope));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (3)),
      arrowExpression);

  SgPlusPlusOp * strideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (getIterationCounterVariableName (3)));

  SgForStatement * forLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (forLoopStatement, block);

  return block;
}

SgStatement *
CPPOpenCLHostSubroutineIndirectLoop::createPlanFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statement to call plan function", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParamaters = buildExprListExp ();

  actualParamaters->append_expression (variableDeclarations->getReference (
      getUserSubroutineName ()));

  actualParamaters->append_expression (variableDeclarations->getReference (
      getOpSetName ()));

  actualParamaters->append_expression (variableDeclarations->getReference (
      getPartitionSizeVariableName (parallelLoop->getUserSubroutineName ())));

  actualParamaters->append_expression (buildIntVal (
      parallelLoop->getNumberOfOpDatArgumentGroups ()));

  actualParamaters->append_expression (variableDeclarations->getReference (
      opDatArray));

  actualParamaters->append_expression (buildIntVal (
      parallelLoop->getNumberOfDistinctIndirectOpDats ()));

  actualParamaters->append_expression (variableDeclarations->getReference (
      indirectionDescriptorArray));

  SgFunctionCallExp * functionCallExpression = buildFunctionCallExp (
      OP2::OP_PLAN_GET, buildVoidType (), actualParamaters, subroutineScope);

  SgExprStatement * assignmentStatement = buildAssignStatement (
      variableDeclarations->getReference (planRet), functionCallExpression);

  return assignmentStatement;
}

void
CPPOpenCLHostSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  appendStatementList (
      createInitialisePlanFunctionArrayStatements ()->getStatementList (),
      subroutineScope);

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
CPPOpenCLHostSubroutineIndirectLoop::createPlanFunctionDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations for plan function",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (
      getIterationCounterVariableName (3),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (3), buildIntType (), subroutineScope));

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

  variableDeclarations->add (blockOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          blockOffset, buildIntType (), subroutineScope));
}

void
CPPOpenCLHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  createOpenCLConfigurationLaunchDeclarations ();

  createPlanFunctionDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}

CPPOpenCLHostSubroutineIndirectLoop::CPPOpenCLHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * kernelSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations,
    CPPUserSubroutine * userSubroutine,
    CPPOpenCLConstantDeclarations * constantDeclarations) :
  CPPOpenCLHostSubroutine (moduleScope, kernelSubroutine, parallelLoop,
      moduleDeclarations, userSubroutine, constantDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating OpenCL host subroutine for indirect loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
