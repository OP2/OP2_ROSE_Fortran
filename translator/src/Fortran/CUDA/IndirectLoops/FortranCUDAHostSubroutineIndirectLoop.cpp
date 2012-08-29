


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


#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranParallelLoop.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranCUDAModuleDeclarations.h>
#include <FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <CompilerGeneratedNames.h>
#include <PlanFunctionNames.h>
#include <OP2.h>
#include <CUDA.h>
#include <Debug.h>
#include <Globals.h>

void
FortranCUDAHostSubroutineIndirectLoop::createKernelFunctionCallStatement (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA kernel function call statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (variableDeclarations->getReference (
      opDatDimensions));

  actualParameters->append_expression (variableDeclarations->getReference (
      opDatCardinalities));

  Debug::getInstance ()->debugMessage ("Adding OP_DAT parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i))
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (getReductionArrayDeviceName (i)));
      }
      else if (parallelLoop->isIndirect (i) || parallelLoop->isDirect (i))
      {
/* Carlo: no more opDatNDevice as arguments */
/*        string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
        
        actualParameters->append_expression (
            moduleDeclarations->getDeclarations()->getReference (getOpDatDeviceName (i) + 
            parallelLoop->getUserSubroutineName () + postfixName));
*/            
      }
      else if (parallelLoop->isRead (i))
      {
        if (parallelLoop->isArray (i))
        {
          actualParameters->append_expression (
              variableDeclarations->getReference (getOpDatDeviceName (i)));
        }
        else
        {
          actualParameters->append_expression (
              variableDeclarations->getReference (getOpDatHostName (i)));
        }
      }
    }
  }

/* Carlo: these variables are now accessible from the module declaration section and they don't need to be passed as parameters */
/*  Debug::getInstance ()->debugMessage (
      "Adding local to global memory remapping parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (getLocalToGlobalMappingName (i)));
      }
    }
  }

  Debug::getInstance ()->debugMessage (
      "Adding global to local memory remapping parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);


  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      actualParameters->append_expression (variableDeclarations->getReference (
          getGlobalToLocalMappingName (i)));
    }
  }
*/
  Debug::getInstance ()->debugMessage ("Adding plan function parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  actualParameters->append_expression (variableDeclarations->getReference (
      pindSizes));

  actualParameters->append_expression (variableDeclarations->getReference (
      pindOffs));

  actualParameters->append_expression (variableDeclarations->getReference (
      pblkMap));

  actualParameters->append_expression (variableDeclarations->getReference (
      poffset));

  actualParameters->append_expression (variableDeclarations->getReference (
      pnelems));

  actualParameters->append_expression (variableDeclarations->getReference (
      pnthrcol));

  actualParameters->append_expression (variableDeclarations->getReference (
      pthrcol));

  actualParameters->append_expression (variableDeclarations->getReference (
      blockOffset));

  SgCudaKernelExecConfig * kernelConfiguration = new SgCudaKernelExecConfig (
      RoseHelper::getFileInfo (), variableDeclarations->getReference (
          CUDA::blocksPerGrid), variableDeclarations->getReference (
          CUDA::threadsPerBlock), variableDeclarations->getReference (
          CUDA::sharedMemorySize), buildNullExpression ());

  kernelConfiguration->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgCudaKernelCallExp * kernelCallExpression = new SgCudaKernelCallExp (
      RoseHelper::getFileInfo (), buildFunctionRefExp (
          calleeSubroutine->getSubroutineName (), subroutineScope),
      actualParameters, kernelConfiguration);

  kernelCallExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  appendStatement (buildExprStatement (kernelCallExpression), scope);
}

void
FortranCUDAHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace LoopVariableNames;
  using namespace OP2::Macros;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating plan function execution statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  /*
   * ======================================================
   * Statement to initialise the block offset
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (blockOffset), buildIntVal (0));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Calling the MPI wait all function if col == ncolors
   * ======================================================
   */
  if ( Globals::getInstance ()->getIncludesMPI () )
  {
    SgBasicBlock * ifBody = buildBasicBlock ();

    appendCallMPIWaitAll (ifBody);    
    
    SgDotExp * planNColorsField = buildDotExp (variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ())),
      buildOpaqueVarRefExp (ncolorsCore, subroutineScope));

    SgExpression * ifGuardExpression = buildEqualityOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      planNColorsField);
      
    SgIfStmt * ifLastColor =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
        ifGuardExpression, ifBody);

    appendStatement (ifLastColor, loopBody);
  }
  
  /*
   * ======================================================
   * Statement to initialise the grid dimension of the
   * CUDA kernel
   * ======================================================
   */

  SgExpression * arrayIndexExpression1 = buildAddOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (1));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (ncolblk), arrayIndexExpression1);

  SgExprStatement * statement1 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::blocksPerGrid),
      arrayExpression1);

  appendStatement (statement1, loopBody);

  /*
   * ======================================================
   * Statement to initialise the number of threads per block
   * in the CUDA kernel: calls a proper function
   * ======================================================
   */
  
  Debug::getInstance ()->debugMessage ("Creating call to block size setting function",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  appendBlockSizeFunctionCall (subroutineScope);

  /*
   * ======================================================
   * Statement to initialise the shared memory size in the
   * CUDA kernel
   * ======================================================
   */

  SgDotExp * dotExpression3 = buildDotExp (variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ())),
      buildOpaqueVarRefExp (nshared, subroutineScope));

  SgExprStatement * statement3 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::sharedMemorySize),
      dotExpression3);

  appendStatement (statement3, loopBody);

  createKernelFunctionCallStatement (loopBody);

  /*
   * ======================================================
   * Statement to synchronise the CUDA threads
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::threadSynchRet),
      CUDA::createHostThreadSynchronisationCallStatement (subroutineScope));

  appendStatement (assignmentStatement2, loopBody);

  /*
   * ======================================================
   * Statement to increment the block offset
   * ======================================================
   */

  SgAddOp * addExpression4 = buildAddOp (variableDeclarations->getReference (
      blockOffset), variableDeclarations->getReference (CUDA::blocksPerGrid));

  SgStatement * statement4 = buildAssignStatement (
      variableDeclarations->getReference (blockOffset), addExpression4);

  appendStatement (statement4, loopBody);

  /*
   * ======================================================
   * The loop starts counting from 0
   * ======================================================
   */

  SgAssignOp * initializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (0));

  /*
   * ======================================================
   * The loop stops counting at the number of colours in
   * the plan, minus one
   * ======================================================
   */

  SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ())),
      buildOpaqueVarRefExp (ncolors, subroutineScope));

  SgExpression * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  appendStatement (loopStatement, subroutineScope);
}

SgBasicBlock *
FortranCUDAHostSubroutineIndirectLoop::createConvertPlanFunctionParametersStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to convert plan function parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgVarRefExp * parameterExpressionA1 = variableDeclarations->getReference (
      getPlanReturnVariableName (parallelLoop->getUserSubroutineName ()));

  SgVarRefExp * parameterExpressionA2 = variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ()));

  SgStatement
      * callStatementA =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionA1, parameterExpressionA2);

  appendStatement (callStatementA, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionE1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          ind_maps, block));

  SgAggregateInitializer * parameterExpressionE3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (numberOfIndirectOpDats));

  SgStatement
      * callStatementE =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionE1,
              variableDeclarations->getReference (pindMaps),
              parameterExpressionE3);

  appendStatement (callStatementE, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionF1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())),
      variableDeclarations->getReference (ncolblk));

  SgDotExp * dotExpressionF3 = buildDotExp (variableDeclarations->getReference (
      set), buildDotExp ( buildOpaqueVarRefExp (Fortran::setPtr, block),
          buildOpaqueVarRefExp (size, block)));

  SgAggregateInitializer * parameterExpressionF3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          dotExpressionF3);

  SgStatement
      * callStatementF =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionF1,
              variableDeclarations->getReference (ncolblk),
              parameterExpressionF3);

  appendStatement (callStatementF, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionG = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nblocks, block));

  SgMultiplyOp * multiplyExpressionG = buildMultiplyOp (dotExpressionG,
      variableDeclarations->getReference (numberOfIndirectOpDats));

  SgExprStatement * assignmentStatementG = buildAssignStatement (
      variableDeclarations->getReference (pindSizesSize), multiplyExpressionG);

  appendStatement (assignmentStatementG, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionG1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          ind_sizes, block));

  SgAggregateInitializer * parameterExpressionG3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pindSizesSize));

  SgStatement
      * callStatementG =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionG1,
              variableDeclarations->getReference (pindSizes),
              parameterExpressionG3);

  appendStatement (callStatementG, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExprStatement * assignmentStatementH = buildAssignStatement (
      variableDeclarations->getReference (pindOffsSize),
      variableDeclarations->getReference (pindSizesSize));

  appendStatement (assignmentStatementH, block);

  SgDotExp * parameterExpressionH1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          ind_offs, block));

  SgAggregateInitializer * parameterExpressionH3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pindOffsSize));

  SgStatement
      * callStatementH =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionH1,
              variableDeclarations->getReference (pindOffs),
              parameterExpressionH3);

  appendStatement (callStatementH, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionI = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nblocks, block));

  SgExprStatement * assignmentStatementI = buildAssignStatement (
      variableDeclarations->getReference (pblkMapSize), dotExpressionI);

  appendStatement (assignmentStatementI, block);

  SgDotExp * parameterExpressionI1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          blkmap, block));

  SgAggregateInitializer * parameterExpressionI3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pblkMapSize));

  SgStatement
      * callStatementI =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionI1,
              variableDeclarations->getReference (pblkMap),
              parameterExpressionI3);

  appendStatement (callStatementI, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionJ = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nblocks, block));

  SgExprStatement * assignmentStatementJ = buildAssignStatement (
      variableDeclarations->getReference (poffsetSize), dotExpressionJ);

  appendStatement (assignmentStatementJ, block);

  SgDotExp * parameterExpressionJ1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          offset, block));

  SgAggregateInitializer * parameterExpressionJ3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (poffsetSize));

  SgStatement
      * callStatementJ =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionJ1,
              variableDeclarations->getReference (poffset),
              parameterExpressionJ3);

  appendStatement (callStatementJ, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionK = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nblocks, block));

  SgExprStatement * assignmentStatementK = buildAssignStatement (
      variableDeclarations->getReference (pnelemsSize), dotExpressionK);

  appendStatement (assignmentStatementK, block);

  SgDotExp * parameterExpressionK1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nelems, block));

  SgAggregateInitializer * parameterExpressionK3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pnelemsSize));

  SgStatement
      * callStatementK =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionK1,
              variableDeclarations->getReference (pnelems),
              parameterExpressionK3);

  appendStatement (callStatementK, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionL = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nblocks, block));

  SgExprStatement * assignmentStatementL = buildAssignStatement (
      variableDeclarations->getReference (pnthrcolSize), dotExpressionL);

  appendStatement (assignmentStatementL, block);

  SgDotExp * parameterExpressionL1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nthrcol, block));

  SgAggregateInitializer * parameterExpressionL3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pnthrcolSize));

  SgStatement
      * callStatementL =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionL1,
              variableDeclarations->getReference (pnthrcol),
              parameterExpressionL3);

  appendStatement (callStatementL, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExprStatement * assignmentStatementM = buildAssignStatement (
      variableDeclarations->getReference (pthrcolSize), buildDotExp (
          variableDeclarations->getReference (set), buildDotExp ( buildOpaqueVarRefExp (Fortran::setPtr, block),
          buildOpaqueVarRefExp (size, block))));

  appendStatement (assignmentStatementM, block);

  SgDotExp * parameterExpressionM1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          thrcol, block));

  SgAggregateInitializer * parameterExpressionM3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pthrcolSize));

  SgStatement
      * callStatementM =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionM1,
              variableDeclarations->getReference (pthrcol),
              parameterExpressionM3);

  appendStatement (callStatementM, block);

  return block;
}

SgBasicBlock *
FortranCUDAHostSubroutineIndirectLoop::createConvertLocalToGlobalMappingStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to convert local-to-global mapping arrays",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionA1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nindirect, block));

  SgAggregateInitializer * parameterExpressionA3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (numberOfIndirectOpDats));

  SgStatement
      * callStatementA =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionA1,
              variableDeclarations->getReference (pnindirect),
              parameterExpressionA3);

  appendStatement (callStatementA, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion calls
   * ======================================================
   */

  unsigned int countIndirectArgs = 1;

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgPntrArrRefExp * parameterExpressionN1 = buildPntrArrRefExp (
            variableDeclarations->getReference (pindMaps), buildIntVal (
                countIndirectArgs));

        SgPntrArrRefExp * parameterExpressionN3 = buildPntrArrRefExp (
            variableDeclarations->getReference (pnindirect), buildIntVal (
                countIndirectArgs));

        SgStatement
            * callStatementN =
                FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                    subroutineScope, parameterExpressionN1,
                    moduleDeclarations->getDeclarations ()->getReference (
                        getLocalToGlobalMappingName (i) + "_" + parallelLoop->getUserSubroutineName () + postfixName), parameterExpressionN3);

        appendStatement (callStatementN, block);

        countIndirectArgs++;
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAHostSubroutineIndirectLoop::createConvertGlobalToLocalMappingStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace OP2VariableNames;
  using std::string;
  
  Debug::getInstance ()->debugMessage (
      "Creating statements to convert global-to-local mapping arrays",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionA1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (maps,
          block));

  SgAggregateInitializer * parameterExpressionA3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (numberOfOpDats));

  SgStatement
      * callStatementA =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionA1,
              variableDeclarations->getReference (pmaps), parameterExpressionA3);

  appendStatement (callStatementA, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion calls
   * ======================================================
   */

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      /*
       * ======================================================
       * Build the body of the if statement
       * ======================================================
       */

      SgBasicBlock * ifBody = buildBasicBlock ();

      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (set),
          buildDotExp ( buildOpaqueVarRefExp (Fortran::setPtr, block),
            buildOpaqueVarRefExp (size, block)));

      SgExprStatement * statement1 = buildAssignStatement (
          variableDeclarations->getReference (getGlobalToLocalMappingSizeName (i)), dotExpression1);

      appendStatement (statement1, ifBody);

      SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (pmaps), buildIntVal (i));

      SgVarRefExp * parameterExpression2 = moduleDeclarations->getDeclarations ()->getReference (
          getGlobalToLocalMappingName (i) + "_" + parallelLoop->getUserSubroutineName () + postfixName);

      SgAggregateInitializer * parameterExpression3 =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->getReference (
                  getGlobalToLocalMappingSizeName (i)));

      SgStatement
          * statement2 =
              FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                  subroutineScope, parameterExpression1, parameterExpression2,
                  parameterExpression3);

      appendStatement (statement2, ifBody);

      /*
       * ======================================================
       * Add the if statement with a NULL else block
       * ======================================================
       */

      SgExpression * arrayIndexExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (indirectionDescriptorArray),
          buildIntVal (i));

      SgExpression * ifGuardExpression = buildGreaterOrEqualOp (
          arrayIndexExpression, buildIntVal (0));

      SgIfStmt * ifStatement =
          RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
              ifGuardExpression, ifBody);

      appendStatement (ifStatement, block);
    }
  }

  return block;
}

void
FortranCUDAHostSubroutineIndirectLoop::createCardinalitiesInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT cardinalities",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  unsigned int countIndirectArgs = 1;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (opDatCardinalities),
            cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
                getLocalToGlobalMappingSizeName (i)));

        SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (pnindirect), buildIntVal (
                countIndirectArgs));

        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression, arrayIndexExpression);

        appendStatement (assignmentStatement, subroutineScope);

        countIndirectArgs++;
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      SgDotExp * fieldSelectionExpression = buildDotExp (
          variableDeclarations->getReference (opDatCardinalities),
          cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
              getGlobalToLocalMappingSizeName (i)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          fieldSelectionExpression, variableDeclarations->getReference (
              getGlobalToLocalMappingSizeName (i)));

      appendStatement (assignmentStatement, subroutineScope);
    }
  }

  vector <string> planFunctionSizeVariables;

  planFunctionSizeVariables.push_back (pblkMapSize);
  planFunctionSizeVariables.push_back (pindOffsSize);
  planFunctionSizeVariables.push_back (pindSizesSize);
  planFunctionSizeVariables.push_back (pnelemsSize);
  planFunctionSizeVariables.push_back (pnthrcolSize);
  planFunctionSizeVariables.push_back (poffsetSize);
  planFunctionSizeVariables.push_back (pthrcolSize);

  for (vector <string>::iterator it = planFunctionSizeVariables.begin (); it
      != planFunctionSizeVariables.end (); ++it)
  {
    SgDotExp * fieldSelectionExpression = buildDotExp (
        variableDeclarations->getReference (opDatCardinalities),
        cardinalitiesDeclaration->getFieldDeclarations ()->getReference (*it));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        fieldSelectionExpression, variableDeclarations->getReference (*it));

    appendStatement (assignmentStatement, subroutineScope);
  }
}

SgExprStatement *
FortranCUDAHostSubroutineIndirectLoop::createPartitionSizeInitialisationFromEnvironment ()
{
  using namespace SageBuilder;
  using namespace SageInterface;  
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace LoopVariableNames;
  using namespace OP2::Macros;
  using namespace OP2::RunTimeVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating call to partition size setting function",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVarRefExp * parameter1 = variableDeclarations->getReference (
      getUserSubroutineName ());

  SgDotExp * parameter2 = buildDotExp (
    variableDeclarations->getReference (getOpSetName ()),
    buildDotExp ( buildOpaqueVarRefExp (Fortran::setPtr, subroutineScope),
      buildOpaqueVarRefExp (size, subroutineScope)));

      
//   SgDotExp * parameter2 = buildDotExp (variableDeclarations->getReference (
//       getOpSetName ()), buildDotExp ( buildOpaqueVarRefExp (OP2::RunTimeVariableNames::size,
//       subroutineScope));


  SgExprListExp * actualParameters = buildExprListExp (parameter1, parameter2);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction (OP2::FortranSpecific::RunTimeFunctions::getPartitionSizeFunctionName,
          subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  SgExprStatement * assignmentStatement = buildAssignStatement (
      variableDeclarations->getReference (planPartitionSize), functionCall);

  return assignmentStatement;
  
}

void
FortranCUDAHostSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  initialiseNumberOfOpArgs (subroutineScope);

  appendPopulationOpArgArray (subroutineScope);
  
  if ( Globals::getInstance ()->getIncludesMPI () )
    appendCallMPIHaloExchangeFunction (subroutineScope);  
  
  createEarlyExitStatementNewLibrary (subroutineScope);

  Debug::getInstance ()->debugMessage (
       "Host subroutine indirect loop, creating profiling declarations",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  initialiseProfilingVariablesDeclaration ();

  createStartTimerHost ();

  appendStatement (createPlanFunctionParametersPreparationStatements (),
      subroutineScope);

  appendStatement (createPartitionSizeInitialisationFromEnvironment (),
      subroutineScope);
      
  appendStatement (createPlanFunctionCallStatement (), subroutineScope);

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  appendStatement (createConvertPlanFunctionParametersStatements (),
      subroutineScope);

  appendStatement (createConvertLocalToGlobalMappingStatements (),
      subroutineScope);

  appendStatement (createConvertGlobalToLocalMappingStatements (),
      subroutineScope);

  /*
   * ======================================================
   * Create reduction prologue. This call must stay
   * after the plan conversion for indirect loops, because
   * it employs nblocks which is contained in the plan 
   * function
   * ======================================================
   */
      
  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }
 
  createCardinalitiesInitialisationStatements ();

  Debug::getInstance ()->debugMessage (
       "Host subroutine indirect loop, creating end timer",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createEndTimerHost ();
  createEndTimerSynchroniseHost ();
  createElapsedTimeHost ();
  createAccumulateTimesHost ();

  Debug::getInstance ()->debugMessage (
       "Host subroutine indirect loop, start time for kernel",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createStartTimerKernel ();

  createPlanFunctionExecutionStatements ();

  createEndTimerKernel ();
  createEndTimerSynchroniseKernel ();
  createElapsedTimeKernel ();
  createAccumulateTimesKernel ();

  createStartTimerHost ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }

  appendStatement (createDeallocateStatements (), subroutineScope);

  Debug::getInstance ()->debugMessage (
       "Host subroutine indirect loop, after deallocation",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);  

  if ( Globals::getInstance ()->getIncludesMPI () )
  {
    appendCallsToMPIReduce (subroutineScope);
    appendCallMPISetDirtyBit (subroutineScope);
  }
  
  createEndTimerHost ();
  createEndTimerSynchroniseHost ();
  createElapsedTimeHost ();
  createAccumulateTimesHost ();
}

void
FortranCUDAHostSubroutineIndirectLoop::createExecutionPlanDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace LoopVariableNames;
  using namespace OP2;
  using std::map;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Creating execution plan variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Create pointer to the execution plan
   * ======================================================
   */

  SgType * op_planType = FortranTypesBuilder::buildClassDeclaration (OP_PLAN,
      subroutineScope)->get_type ();

  variableDeclarations->add (getActualPlanVariableName (
      parallelLoop->getUserSubroutineName ()),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getActualPlanVariableName (parallelLoop->getUserSubroutineName ()),
          buildPointerType (op_planType), subroutineScope));

  /*
   * ======================================================
   * Create pointer to the array containing arrays of
   * local to global mappings
   * ======================================================
   */

  SgType * c_devptrType = FortranTypesBuilder::buildClassDeclaration (
      "c_devptr", subroutineScope)->get_type ();

  variableDeclarations->add (pindMaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          pindMaps, buildPointerType (FortranTypesBuilder::getArray_RankOne (
              c_devptrType)), subroutineScope));

  variableDeclarations->add (pmaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (pmaps,
          buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope));

  variableDeclarations->add (pindMapsSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          pindMapsSize, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

/* Carlo: declarations removed because they are now declared in the module declaration section */          
//   for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
//   {
//     if (parallelLoop->isDuplicateOpDat (i) == false)
//     {
//       if (parallelLoop->isIndirect (i))
//       {
//         string const variableName = getLocalToGlobalMappingName (i);
// 
//         variableDeclarations->add (variableName,
//             FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
//                 variableName, FortranTypesBuilder::getArray_RankOne (
//                     FortranTypesBuilder::getFourByteInteger ()),
//                 subroutineScope, 2, CUDA_DEVICE, ALLOCATABLE));
//       }
//     }
//   }

  /*
   * ======================================================
   * Create arrays for OP_DAT, OP_INDIRECTION,
   * OP_MAP, OP_ACCESS arguments. These arrays are filled up
   * with the actual values of the OP_DAT, OP_INDIRECTION,
   * OP_MAP, OP_ACCESS that are passed to the OP_PAR_LOOP;
   * these arrays are then given to the plan function.
   * Note, therefore, that the size of the arrays is exactly
   * the number of OP_DAT argument groups.
   * There is an additional array 'inds' storing which
   * OP_DAT arguments are accessed through an indirection
   * ======================================================
   */

  vector <string> fourByteIntegerArrays;

  fourByteIntegerArrays.push_back (opDatArray);
  fourByteIntegerArrays.push_back (mappingIndicesArray);
  fourByteIntegerArrays.push_back (mappingArray);
  fourByteIntegerArrays.push_back (accessDescriptorArray);
  fourByteIntegerArrays.push_back (indirectionDescriptorArray);

  for (vector <string>::iterator it = fourByteIntegerArrays.begin (); it
      != fourByteIntegerArrays.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger (), 1,
                parallelLoop->getNumberOfOpDatArgumentGroups ()),
            subroutineScope));
  }

  /*
   * ======================================================
   * Create variables modelling the indirect mappings to
   * local indices in shared memory
   * ======================================================
   */
/* Carlo: declarations removed because they are now in the module declaration section */
/*
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      string const variableName = getGlobalToLocalMappingName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getArray_RankOne (
                  FortranTypesBuilder::getTwoByteInteger ()), subroutineScope,
              2, CUDA_DEVICE, ALLOCATABLE));
    }
  }*/

  /*
   * ======================================================
   * Create variables modelling the number of elements
   * in each block of an indirect mapping
   * ======================================================
   */
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      string const variableName = getGlobalToLocalMappingSizeName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of integer fields. These fields need to be accessed
   * on the Fortran side, so create local variables that
   * enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> fourByteIntegerVariables;

  // Carlo: already declared in common ancestor of direct and indirect loop
/*  fourByteIntegerVariables.push_back (getIterationCounterVariableName (1));
  fourByteIntegerVariables.push_back (getIterationCounterVariableName (2));  */
  
  fourByteIntegerVariables.push_back (numberOfIndirectOpDats);
  fourByteIntegerVariables.push_back (blockOffset);
  fourByteIntegerVariables.push_back (pindSizesSize);
  fourByteIntegerVariables.push_back (pindOffsSize);
  fourByteIntegerVariables.push_back (pblkMapSize);
  fourByteIntegerVariables.push_back (poffsetSize);
  fourByteIntegerVariables.push_back (pnelemsSize);
  fourByteIntegerVariables.push_back (pnthrcolSize);
  fourByteIntegerVariables.push_back (pthrcolSize);

  for (vector <string>::iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of integer array fields. These fields need to be
   * accessed on the Fortran side ON THE HOST, so create local
   * variables that enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> integerPointerVariables;

  integerPointerVariables.push_back (ncolblk);
  integerPointerVariables.push_back (pnindirect);

  for (vector <string>::iterator it = integerPointerVariables.begin (); it
      != integerPointerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildPointerType (FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ())), subroutineScope));
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of array integer fields. These fields need to be accessed
   * on the Fortran side ON THE DEVICE, so create local variables
   * that enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> deviceIntegerArrayVariables;

  deviceIntegerArrayVariables.push_back (pindSizes);
  deviceIntegerArrayVariables.push_back (pindOffs);
  deviceIntegerArrayVariables.push_back (pblkMap);
  deviceIntegerArrayVariables.push_back (poffset);
  deviceIntegerArrayVariables.push_back (pnelems);
  deviceIntegerArrayVariables.push_back (pnthrcol);
  deviceIntegerArrayVariables.push_back (pthrcol);

  for (vector <string>::iterator it = deviceIntegerArrayVariables.begin (); it
      != deviceIntegerArrayVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ()), subroutineScope,
            2, CUDA_DEVICE, ALLOCATABLE));
  }
  
  /*
   * ======================================================
   * The partition size and block size are part of the
   * information needed to execute a plan, hence they
   * are declared here
   * ======================================================
   */
  vector <string> partitionAndBlockSizes;

  partitionAndBlockSizes.push_back (partitionSize);
  partitionAndBlockSizes.push_back (blockSize);

  for (vector <string>::iterator it = partitionAndBlockSizes.begin (); it
      != partitionAndBlockSizes.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }
}

void
FortranCUDAHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  /*
   * ======================================================
   * Declare array of op_args and variable with number
   * of parameters
   * ======================================================
   */
  createCommonLocalVariableDeclarations (subroutineScope);
  
  createOpDatDimensionsDeclaration ();

  createOpDatCardinalitiesDeclaration ();

  createDataMarshallingDeclarations ();  
  
  createCUDAConfigurationLaunchDeclarations ();

  createExecutionPlanDeclarations ();

  createIterationVariablesDeclarations ();

  createDumpOfOutputDeclarations (subroutineScope);
  
  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }

  /*
   * ======================================================
   * Profiling declarations. Eventually, this should only
   * be done if a certain compiler option is turned on
   * ======================================================
   */

  createProfilingVariablesDeclaration ();
}

FortranCUDAHostSubroutineIndirectLoop::FortranCUDAHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    FortranKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop * cardinalitiesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranCUDAHostSubroutine (moduleScope, kernelSubroutine, parallelLoop,
      cardinalitiesDeclaration, dimensionsDeclaration, moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "CUDA host subroutine creation for indirect loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  variableDeclarations->addVisibilityToSymbolsFromOuterScope (
      moduleDeclarations->getDeclarations ());

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
