


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


#include "CPPCUDAHostSubroutineDirectLoop.h"
#include "CPPParallelLoop.h"
#include "CPPKernelSubroutine.h"
#include "CPPModuleDeclarations.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "RoseHelper.h"
#include "CompilerGeneratedNames.h"
#include "CUDA.h"
#include "OP2.h"

void
CPPCUDAHostSubroutineDirectLoop::createKernelFunctionCallStatement (
    SgScopeStatement * scope)
{
  using namespace SageInterface;
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statement to call CUDA kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i) || parallelLoop->isReductionRequired (i))
      {
        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (getOpDatName (dat_num)),
            buildOpaqueVarRefExp (data_d, subroutineScope));

        SgCastExp * castExpression = buildCastExp (dotExpression,
            buildPointerType (parallelLoop->getOpDatBaseType (i)));

        actualParameters->append_expression (castExpression);
      }
    }
  }

  actualParameters->append_expression (variableDeclarations->getReference (
      sharedMemoryOffset));

  SgArrowExp * arrowExpression = buildArrowExp (
      variableDeclarations->getReference (getOpSetName ()),
      buildOpaqueVarRefExp (size, subroutineScope));

  actualParameters->append_expression (arrowExpression);

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

void
CPPCUDAHostSubroutineDirectLoop::createCUDAKernelInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::Macros;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA kernel initialisation statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::blocksPerGrid), buildIntVal (
          200));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::threadsPerBlock),
      variableDeclarations->getReference (getBlockSizeVariableName (
          parallelLoop->getUserSubroutineName ())));

  appendStatement (assignmentStatement2, subroutineScope);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::sharedMemorySize), buildIntVal (
          0));

  appendStatement (assignmentStatement3, subroutineScope);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i))
      {
        SgSizeOfOp * sizeOfExpression = buildSizeOfOp (
            parallelLoop->getOpDatBaseType (i));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (sizeOfExpression,
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgFunctionCallExp * maxCallExpression =
            OP2::Macros::createMaxCallStatement (subroutineScope,
                variableDeclarations->getReference (CUDA::sharedMemorySize),
                multiplyExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (CUDA::sharedMemorySize),
            maxCallExpression);

        appendStatement (assignmentStatement, subroutineScope);
      }
    }
  }

  SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (
      variableDeclarations->getReference (CUDA::sharedMemorySize),
      buildOpaqueVarRefExp (warpSizeMacro, subroutineScope));

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (sharedMemoryOffset),
      multiplyExpression4);

  appendStatement (assignmentStatement4, subroutineScope);

  SgMultiplyOp * multiplyExpression5 = buildMultiplyOp (
      variableDeclarations->getReference (CUDA::sharedMemorySize),
      variableDeclarations->getReference (CUDA::threadsPerBlock));

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::sharedMemorySize),
      multiplyExpression5);

  appendStatement (assignmentStatement5, subroutineScope);
}

void
CPPCUDAHostSubroutineDirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createCUDAKernelInitialisationStatements ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  createKernelFunctionCallStatement (subroutineScope);

  SgFunctionCallExp
      * threadSynchronisationExpression =
          CUDA::OP2RuntimeSupport::getCUDASafeHostThreadSynchronisationCallStatement (
              subroutineScope);

  appendStatement (buildExprStatement (threadSynchronisationExpression),
      subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPCUDAHostSubroutineDirectLoop::createCUDAKernelActualParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (sharedMemoryOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          sharedMemoryOffset, buildIntType (), subroutineScope));
}

void
CPPCUDAHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  createCUDAConfigurationLaunchDeclarations ();

  createCUDAKernelActualParameterDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}

CPPCUDAHostSubroutineDirectLoop::CPPCUDAHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope, CPPCUDAKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations) :
  CPPCUDAHostSubroutine (moduleScope, calleeSubroutine, parallelLoop,
      moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
