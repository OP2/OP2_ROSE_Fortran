


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
#include "OP2Definitions.h"
#include "CPPProgramDeclarationsAndDefinitions.h"

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

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i))
    {
      /*
       * FIXME, this should be pulled out into a generic routine so
       * that one can use different matrix storage formats that
       * require different arguments to the device kernel.
       */

      /* Pass op_mat data pointer */
      unsigned int mat_num = parallelLoop->getOpMatArgNum (i);
      OpArgMatDefinition * arg_mat = parallelLoop->getOpMatArg (mat_num);
      OpMatDefinition * mat = declarations->getOpMatDefinition (arg_mat->getMatName ());

      SgDotExp * data = buildDotExp (
        variableDeclarations->getReference (getOpMatName (mat_num)),
        buildOpaqueVarRefExp (data_d, subroutineScope));

      SgCastExp * castExpression = buildCastExp (data,
          buildPointerType (mat->getBaseType ()));
      actualParameters->append_expression (castExpression);

      /* pass row pointer */
      SgDotExp * rowptr = buildDotExp (
        variableDeclarations->getReference (getOpMatName (mat_num)),
        buildOpaqueVarRefExp (rowptr_d, subroutineScope));
      actualParameters->append_expression (rowptr);
      /* pass col pointer */
      SgDotExp * colptr = buildDotExp (
        variableDeclarations->getReference (getOpMatName (mat_num)),
        buildOpaqueVarRefExp (colptr_d, subroutineScope));
      actualParameters->append_expression (colptr);
      /* pass nrow */
      SgDotExp * nrow_d = buildDotExp (
        variableDeclarations->getReference (getOpMatName (mat_num)),
        buildOpaqueVarRefExp (nrow, subroutineScope));
      actualParameters->append_expression (nrow_d);
    }
    else {
      unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
      if (parallelLoop->isDuplicateOpDat (i) == false)
      {
        SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (getOpDatName (dat_num)),
          buildOpaqueVarRefExp (data_d, subroutineScope));

        SgCastExp * castExpression = buildCastExp (
          dotExpression,
          buildPointerType (parallelLoop->getOpDatBaseType (i)));

        actualParameters->append_expression (castExpression);
      }
    }
  }

  unsigned int arrayIndex = 0;
  unsigned int matIndex = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i))
    {
      SgPntrArrRefExp * expr = buildPntrArrRefExp (
        buildOpaqueVarRefExp (mat_maps, subroutineScope), buildIntVal (
          matIndex));
      SgArrowExp * arrow = buildArrowExp (
        variableDeclarations->getReference (planRet), expr);
      actualParameters->append_expression (arrow);

      SgPntrArrRefExp * expr2 = buildPntrArrRefExp (
        buildOpaqueVarRefExp (mat_maps2, subroutineScope), buildIntVal (
          matIndex));
      SgArrowExp * arrow2 = buildArrowExp (
        variableDeclarations->getReference (planRet), expr2);
      actualParameters->append_expression (arrow2);
      matIndex++;
    }
    else
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
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isIndirect (i))
    {
      SgPntrArrRefExp * arrayExpression =
          buildPntrArrRefExp (buildOpaqueVarRefExp (loc_maps, subroutineScope),
              buildIntVal (dat_num - 1));

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

  SgArrowExp * arrowExpression3 = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          nshared, subroutineScope));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::sharedMemorySize),
      arrowExpression3);

  appendStatement (assignmentStatement3, loopBody);

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
      parallelLoop->getNumberOfArgumentGroups ()));

  actualParamaters->append_expression (variableDeclarations->getReference (
      opDatArray));

  actualParamaters->append_expression (buildIntVal (
      parallelLoop->getNumberOfDistinctIndirectOpDats () +
      parallelLoop->getNumberOfOpMatArgumentGroups ()));

  actualParamaters->append_expression (variableDeclarations->getReference (
      indirectionDescriptorArray));

  SgFunctionCallExp * functionCallExpression = buildFunctionCallExp (
      OP2::OP_PLAN_GET, buildVoidType (), actualParamaters, subroutineScope);

  SgExprStatement * assignmentStatement = buildAssignStatement (
      variableDeclarations->getReference (planRet), functionCallExpression);

  return assignmentStatement;
}

void
CPPCUDAHostSubroutineIndirectLoop::createStatements ()
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
              parallelLoop->getNumberOfArgumentGroups ())),
          subroutineScope));

  variableDeclarations->add (indirectionDescriptorArray,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          indirectionDescriptorArray, buildArrayType (buildIntType (),
              buildIntVal (parallelLoop->getNumberOfArgumentGroups ())),
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
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations,
    CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPCUDAHostSubroutine (moduleScope, calleeSubroutine, parallelLoop,
      moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  this->declarations = declarations;

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
