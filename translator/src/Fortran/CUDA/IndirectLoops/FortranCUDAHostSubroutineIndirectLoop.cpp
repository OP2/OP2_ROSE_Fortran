#include "FortranCUDAHostSubroutineIndirectLoop.h"
#include "FortranParallelLoop.h"
#include "FortranOpDatDimensionsDeclaration.h"
#include "FortranCUDAModuleDeclarations.h"
#include "FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop.h"
#include "FortranTypesBuilder.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "RoseHelper.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "OP2.h"
#include "CUDA.h"
#include "Debug.h"

SgStatement *
FortranCUDAHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace PlanFunctionVariableNames;

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
        actualParameters->append_expression (
            variableDeclarations->getReference (getOpDatDeviceName (i)));
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

  Debug::getInstance ()->debugMessage (
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

  return buildExprStatement (kernelCallExpression);
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
   * in the CUDA kernel
   * ======================================================
   */

  SgExprStatement * statement2 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::threadsPerBlock),
      buildOpaqueVarRefExp (threadBlockSizeMacro, subroutineScope));

  appendStatement (statement2, loopBody);

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

  appendStatement (createKernelFunctionCallStatement (), loopBody);

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

  SgDotExp * parameterExpressionF3 = buildDotExp (
      variableDeclarations->getReference (set), buildOpaqueVarRefExp (size,
          block));

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
          variableDeclarations->getReference (set), buildOpaqueVarRefExp (size,
              block)));

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
                    variableDeclarations->getReference (
                        getLocalToGlobalMappingName (i)), parameterExpressionN3);

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
          variableDeclarations->getReference (set), buildOpaqueVarRefExp (size,
              block));

      SgExprStatement * statement1 = buildAssignStatement (
          variableDeclarations->getReference (getGlobalToLocalMappingSizeName (
              i)), dotExpression1);

      appendStatement (statement1, ifBody);

      SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (pmaps), buildIntVal (i));

      SgVarRefExp * parameterExpression2 = variableDeclarations->getReference (
          getGlobalToLocalMappingName (i));

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

SgBasicBlock *
FortranCUDAHostSubroutineIndirectLoop::createPlanFunctionParametersPreparationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to prepare plan function parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpDatName (i)), buildOpaqueVarRefExp (
        OP2::RunTimeVariableNames::index, block));

    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (opDatArray), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, dotExpression);

    appendStatement (assignmentStatement, block);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (mappingIndicesArray), buildIntVal (
            i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, variableDeclarations->getReference (
            getOpIndirectionName (i)));

    appendStatement (assignmentStatement, block);
  }

  SgExpression * initializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (1));

  SgExpression * upperBoundExpression = buildIntVal (
      parallelLoop->getNumberOfOpDatArgumentGroups ());

  /*
   * ======================================================
   * Build the body of the do-loop
   * ======================================================
   */

  SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (mappingIndicesArray),
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgSubtractOp * subtractExpression1 = buildSubtractOp (arrayIndexExpression1,
      buildIntVal (1));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayIndexExpression1, subtractExpression1);

  SgBasicBlock * ifBody = buildBasicBlock (assignmentStatement1);

  SgExpression * ifGuardExpression = buildNotEqualOp (arrayIndexExpression1,
      buildIntVal (-1));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  SgBasicBlock * loopBody = buildBasicBlock (ifStatement);

  SgFortranDo * fortranDoStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  appendStatement (fortranDoStatement, block);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpMapName (i)), buildOpaqueVarRefExp (
        OP2::RunTimeVariableNames::index, block));

    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (mappingArray), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, dotExpression);

    appendStatement (assignmentStatement, block);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (accessDescriptorArray),
        buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, variableDeclarations->getReference (
            getOpAccessName (i)));

    appendStatement (assignmentStatement, block);
  }

  /*
   * ======================================================
   * Set up a mapping between OP_DATs and indirection
   * values. At the beginning everything is set to undefined
   * ======================================================
   */
  int const undefinedIndex = -2;

  std::map <std::string, int> indexValues;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      indexValues[parallelLoop->getOpDatVariableName (i)] = undefinedIndex;
    }
  }

  /*
   * ======================================================
   * Start at the value defined by Mike Giles in his
   * implementation
   * ======================================================
   */
  unsigned int nextIndex = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (indirectionDescriptorArray),
        buildIntVal (i));

    SgExprStatement * assignmentStatement;

    if (parallelLoop->isIndirect (i))
    {
      if (indexValues[parallelLoop->getOpDatVariableName (i)] == undefinedIndex)
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
            buildIntVal (nextIndex));

        indexValues[parallelLoop->getOpDatVariableName (i)] = nextIndex;

        nextIndex++;
      }
      else
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
            buildIntVal (indexValues[parallelLoop->getOpDatVariableName (i)]));
      }
    }
    else
    {
      assignmentStatement = buildAssignStatement (arrayIndexExpression,
          buildIntVal (-1));
    }

    appendStatement (assignmentStatement, block);
  }

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (numberOfOpDats), buildIntVal (
          parallelLoop->getNumberOfOpDatArgumentGroups ()));

  appendStatement (assignmentStatement2, block);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (numberOfIndirectOpDats), buildIntVal (
          parallelLoop->getNumberOfDistinctIndirectOpDats ()));

  appendStatement (assignmentStatement3, block);

  return block;
}

SgExprStatement *
FortranCUDAHostSubroutineIndirectLoop::createPlanFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage ("Creating plan function call statement",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVarRefExp * parameter1 = variableDeclarations->getReference (
      getUserSubroutineName ());

  SgDotExp * parameter2 = buildDotExp (variableDeclarations->getReference (
      getOpSetName ()), buildOpaqueVarRefExp (OP2::RunTimeVariableNames::index,
      subroutineScope));

  SgVarRefExp * parameter3 =
      variableDeclarations->getReference (numberOfOpDats);

  SgVarRefExp * parameter4 = variableDeclarations->getReference (opDatArray);

  SgVarRefExp * parameter5 = variableDeclarations->getReference (
      mappingIndicesArray);

  SgVarRefExp * parameter6 = variableDeclarations->getReference (mappingArray);

  SgVarRefExp * parameter7 = variableDeclarations->getReference (
      accessDescriptorArray);

  SgVarRefExp * parameter8 = variableDeclarations->getReference (
      numberOfIndirectOpDats);

  SgVarRefExp * parameter9 = variableDeclarations->getReference (
      indirectionDescriptorArray);

  SgExprListExp * actualParameters = buildExprListExp (parameter1, parameter2,
      parameter3, parameter4, parameter5, parameter6, parameter7, parameter8,
      parameter9);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction (CUDA::fortranCplanFunction,
          subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  SgExprStatement * assignmentStatement = buildAssignStatement (
      variableDeclarations->getReference (getPlanReturnVariableName (
          parallelLoop->getUserSubroutineName ())), functionCall);

  return assignmentStatement;
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

void
FortranCUDAHostSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  appendStatement (createPlanFunctionParametersPreparationStatements (),
      subroutineScope);

  appendStatement (createPlanFunctionCallStatement (), subroutineScope);

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  appendStatement (createConvertPlanFunctionParametersStatements (),
      subroutineScope);

  appendStatement (createConvertLocalToGlobalMappingStatements (),
      subroutineScope);

  appendStatement (createConvertGlobalToLocalMappingStatements (),
      subroutineScope);

  createCardinalitiesInitialisationStatements ();

  createPlanFunctionExecutionStatements ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }

  appendStatement (createDeallocateStatements (), subroutineScope);
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

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const variableName = getLocalToGlobalMappingName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getArray_RankOne (
                    FortranTypesBuilder::getFourByteInteger ()),
                subroutineScope, 2, CUDA_DEVICE, ALLOCATABLE));
      }
    }
  }

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
  }

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

  fourByteIntegerVariables.push_back (getIterationCounterVariableName (1));
  fourByteIntegerVariables.push_back (getIterationCounterVariableName (2));
  fourByteIntegerVariables.push_back (numberOfOpDats);
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
}

void
FortranCUDAHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  createOpDatDimensionsDeclaration ();

  createOpDatCardinalitiesDeclaration ();

  createDataMarshallingDeclarations ();

  createCUDAConfigurationLaunchDeclarations ();

  createExecutionPlanDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
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
