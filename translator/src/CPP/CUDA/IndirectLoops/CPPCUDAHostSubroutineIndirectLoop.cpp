


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


#include "CPPCUDAHostSubroutineIndirectLoop.h"
#include "CPPModuleDeclarations.h"
#include "RoseHelper.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "CUDA.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "OP2.h"

void
CPPCUDAHostSubroutineIndirectLoop::createKernelFunctionCallStatement (
    SgScopeStatement * scope)
{
  using namespace SageInterface;
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statement to call CUDA kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (data_d, subroutineScope));

      SgCastExp * castExpression = buildCastExp (dotExpression,
          buildPointerType (parallelLoop->getOpDatBaseType (i)));

      actualParameters->append_expression (castExpression);
    }
  }

  unsigned int arrayIndex = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            buildOpaqueVarRefExp (ind_maps, subroutineScope), buildIntVal (
                arrayIndex));

        SgArrowExp * arrowExpression = buildArrowExp (
            variableDeclarations->getReference (planRet), arrayExpression);

        actualParameters->append_expression (arrowExpression);

        arrayIndex++;
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      SgPntrArrRefExp * arrayExpression =
          buildPntrArrRefExp (buildOpaqueVarRefExp (loc_maps, subroutineScope),
              buildIntVal (i - 1));

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
          nthrcol, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          thrcol, subroutineScope)));

  actualParameters->append_expression (variableDeclarations->getReference (
      blockOffset));

  SgCudaKernelExecConfig * kernelConfiguration = new SgCudaKernelExecConfig (
      RoseHelper::getFileInfo (), variableDeclarations->getReference (
          CUDA::blocksPerGrid), variableDeclarations->getReference (
          CUDA::threadsPerBlock), variableDeclarations->getReference (
          CUDA::sharedMemorySize));

  kernelConfiguration->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgCudaKernelCallExp * kernelCallExpression = new SgCudaKernelCallExp (
      RoseHelper::getFileInfo (), buildFunctionRefExp (
          calleeSubroutine->getSubroutineName (), subroutineScope),
      actualParameters, kernelConfiguration);

  kernelCallExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  appendStatement (buildExprStatement (kernelCallExpression), scope);
}

SgBasicBlock *
CPPCUDAHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace ReductionVariableNames;
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
      variableDeclarations->getReference (CUDA::blocksPerGrid),
      arrowExpression2);

  appendStatement (assignmentStatement2, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::threadsPerBlock),
      variableDeclarations->getReference (getBlockSizeVariableName (
          parallelLoop->getUserSubroutineName ())));

  appendStatement (assignmentStatement4, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgArrowExp * arrowExpression3 = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          nshared, subroutineScope));

  SgVarRefExp * smem = variableDeclarations->getReference (CUDA::sharedMemorySize);

  SgExprStatement * assignmentStatement3;
  if (parallelLoop->isReductionRequired())
  {
    SgExprListExp * parms = buildExprListExp ();
    parms->append_expression (arrowExpression3);
    parms->append_expression (buildMultiplyOp (variableDeclarations->getReference (reductionSharedMemorySize),
                                               variableDeclarations->getReference (CUDA::threadsPerBlock)));
    assignmentStatement3 = buildAssignStatement (smem,
      buildFunctionCallExp ("MAX", buildVoidType (), parms, subroutineScope));
  }
  else
  {
    assignmentStatement3 = buildAssignStatement (smem, arrowExpression3);
  }
  appendStatement (assignmentStatement3, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  createKernelFunctionCallStatement (loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgFunctionCallExp
      * threadSynchronizeExpression =
          CUDA::OP2RuntimeSupport::getCUDASafeHostThreadSynchronisationCallStatement (
              subroutineScope);

  appendStatement (buildExprStatement (threadSynchronizeExpression), loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement5 = buildExprStatement (
      buildPlusAssignOp (variableDeclarations->getReference (blockOffset),
          variableDeclarations->getReference (CUDA::blocksPerGrid)));

  appendStatement (assignmentStatement5, loopBody);

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
CPPCUDAHostSubroutineIndirectLoop::createPlanFunctionCallStatement ()
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
CPPCUDAHostSubroutineIndirectLoop::createSetMaxBlocksPerGrid ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;

  SgVarRefExp * itvar = variableDeclarations->getReference (getIterationCounterVariableName (1));
  SgExprStatement * init = buildAssignStatement (
    itvar,
    buildIntVal (0));
  SgArrowExp * ncol = buildArrowExp (
    variableDeclarations->getReference (planRet),
    buildOpaqueVarRefExp (ncolors, subroutineScope));
  SgLessThanOp * test = buildLessThanOp (itvar, ncol);

  SgPlusPlusOp * increment = buildPlusPlusOp (itvar);

  SgBasicBlock * body = buildBasicBlock ();

  SgVarRefExp * blocks = variableDeclarations->getReference (CUDA::blocksPerGrid);

  // Zero blocksPerGrid
  appendStatement(buildAssignStatement (blocks, buildIntVal (0)),
                  subroutineScope);


  SgExprListExp * parms = buildExprListExp ();
  parms->append_expression (blocks);
  SgPntrArrRefExp * nblk = buildPntrArrRefExp (
      buildOpaqueVarRefExp (ncolblk, subroutineScope),
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgArrowExp * nblocks = buildArrowExp (
    variableDeclarations->getReference (planRet),
    nblk);
  appendStatement (
    buildAssignStatement (variableDeclarations->getReference (getIterationCounterVariableName (2)),
                          nblocks), body);
  parms->append_expression (variableDeclarations->getReference (getIterationCounterVariableName (2)));

  SgExprStatement * assign = buildAssignStatement (
    blocks,
    buildFunctionCallExp ("MAX", buildVoidType (), parms, subroutineScope));

  appendStatement (assign, body);
  SgForStatement * forLoopStatement = buildForStatement (
    init, buildExprStatement (test),
    increment, body);

  appendStatement (forLoopStatement, subroutineScope);
}

void
CPPCUDAHostSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  appendStatementList (
      createInitialisePlanFunctionArrayStatements ()->getStatementList (),
      subroutineScope);

  appendStatement (createPlanFunctionCallStatement (), subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createSetMaxBlocksPerGrid ();
    createReductionPrologueStatements ();
  }

  appendStatementList (
      createPlanFunctionExecutionStatements ()->getStatementList (),
      subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPCUDAHostSubroutineIndirectLoop::createPlanFunctionDeclarations ()
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
CPPCUDAHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  createCUDAConfigurationLaunchDeclarations ();

  createPlanFunctionDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}

CPPCUDAHostSubroutineIndirectLoop::CPPCUDAHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope, CPPCUDAKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations) :
  CPPCUDAHostSubroutine (moduleScope, calleeSubroutine, parallelLoop,
      moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
