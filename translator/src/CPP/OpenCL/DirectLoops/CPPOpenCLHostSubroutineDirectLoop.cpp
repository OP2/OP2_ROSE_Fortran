


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


#include "CPPOpenCLHostSubroutineDirectLoop.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "RoseHelper.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include "OpenCL.h"
#include "OP2.h"

void
CPPOpenCLHostSubroutineDirectLoop::createKernelFunctionCallStatement (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to call OpenCL kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Assign the kernel pointer using the run-time support
   * for OpenCL
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::kernelPointer),
      OpenCL::OP2RuntimeSupport::getKernel (scope,
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
      if (parallelLoop->isDirect (i))
      {
        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (getOpDatName (i)),
            buildOpaqueVarRefExp (OP2::RunTimeVariableNames::data_d, scope));

        SgFunctionCallExp * kernelArgumentExpression =
            OpenCL::getSetKernelArgumentCallExpression (scope,
                variableDeclarations->getReference (OpenCL::kernelPointer),
                argumentCounter, OpenCL::getMemoryType (scope), dotExpression);

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

        argumentCounter++;
      }
    }
  }

  /*
   * ======================================================
   * OpenCL kernel argument for shared memory offset
   * ======================================================
   */
  SgFunctionCallExp * kernelArgumentExpression2 =
      OpenCL::getSetKernelArgumentCallExpression (scope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter, buildIntType (), variableDeclarations->getReference (
              sharedMemoryOffset));

  SgBitOrOp * orExpression2 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression2);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression2);

  appendStatement (assignmentStatement2, scope);

  argumentCounter++;

  /*
   * ======================================================
   * OpenCL kernel argument for set size
   * ======================================================
   */

  SgArrowExp * arrowExpression3 = buildArrowExp (
      variableDeclarations->getReference (getOpSetName ()),
      buildOpaqueVarRefExp (OP2::RunTimeVariableNames::size, scope));

  SgFunctionCallExp * kernelArgumentExpression3 =
      OpenCL::getSetKernelArgumentCallExpression (scope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter, buildIntType (), arrowExpression3);

  SgBitOrOp * orExpression3 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression3);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression3);

  appendStatement (assignmentStatement3, scope);

  argumentCounter++;

  /*
   * ======================================================
   * OpenCL kernel argument for dynamic shared memory size
   * ======================================================
   */

  SgFunctionCallExp * kernelArgumentExpression4 =
      OpenCL::getSetKernelArgumentCallExpression (scope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter, OpenCL::getSizeType (scope),
          variableDeclarations->getReference (OpenCL::sharedMemorySize));

  SgBitOrOp * orExpression4 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression4);

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression4);

  appendStatement (assignmentStatement4, scope);

  /*
   * ======================================================
   * OP_DECL_CONST actual parameters
   * ======================================================
   */

  addOpDeclConstActualParameters (scope, argumentCounter);
}

void
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::Macros;

  Debug::getInstance ()->debugMessage (
      "Creating OpenCL kernel initialisation statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Setting blocks per grid
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::blocksPerGrid), buildIntVal (
          200));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Setting threads per block
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::threadsPerBlock),
      variableDeclarations->getReference (getBlockSizeVariableName (
          parallelLoop->getUserSubroutineName ())));

  appendStatement (assignmentStatement2, subroutineScope);

  /*
   * ======================================================
   * Setting total number of threads
   * ======================================================
   */

  SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (
      variableDeclarations->getReference (OpenCL::threadsPerBlock),
      variableDeclarations->getReference (OpenCL::blocksPerGrid));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::totalThreadNumber),
      multiplyExpression3);

  appendStatement (assignmentStatement3, subroutineScope);

  /*
   * ======================================================
   * Setting shared memory size
   * ======================================================
   */

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      buildIntVal (0));

  appendStatement (assignmentStatement4, subroutineScope);

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
            buildIntVal (parallelLoop->getSizeOfOpDat (i)));

        SgFunctionCallExp * maxCallExpression =
            OP2::Macros::createMaxCallStatement (subroutineScope,
                variableDeclarations->getReference (OpenCL::sharedMemorySize),
                multiplyExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (OpenCL::sharedMemorySize),
            maxCallExpression);

        appendStatement (assignmentStatement, subroutineScope);
      }
    }
  }

  SgMultiplyOp * multiplyExpression5 = buildMultiplyOp (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      buildOpaqueVarRefExp (warpSizeMacro, subroutineScope));

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      variableDeclarations->getReference (sharedMemoryOffset),
      multiplyExpression5);

  appendStatement (assignmentStatement5, subroutineScope);

  SgMultiplyOp * multiplyExpression6 = buildMultiplyOp (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      variableDeclarations->getReference (OpenCL::threadsPerBlock));

  SgExprStatement * assignmentStatement6 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      multiplyExpression6);

  appendStatement (assignmentStatement6, subroutineScope);
}

void
CPPOpenCLHostSubroutineDirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createOpenCLKernelInitialisationStatements ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  createKernelFunctionCallStatement (subroutineScope);

  createKernelCallEpilogueStatements (subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelActualParameterDeclarations ()
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
CPPOpenCLHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  createOpenCLConfigurationLaunchDeclarations ();

  createOpenCLKernelActualParameterDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}

CPPOpenCLHostSubroutineDirectLoop::CPPOpenCLHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations,
    CPPUserSubroutine * userSubroutine,
    CPPOpenCLConstantDeclarations * constantDeclarations) :
  CPPOpenCLHostSubroutine (moduleScope, calleeSubroutine, parallelLoop,
      moduleDeclarations, userSubroutine, constantDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
