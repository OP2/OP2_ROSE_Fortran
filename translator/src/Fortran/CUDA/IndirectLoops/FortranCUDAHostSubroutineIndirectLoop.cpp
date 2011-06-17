#include <Debug.h>
#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <ROSEHelper.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranCUDAHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildVarRefExp;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          IndirectAndDirectLoop::Fortran::VariableNames::argsSizes)));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      actualParameters->append_expression (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatDeviceName (i))));

      actualParameters->append_expression (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getLocalToGlobalMappingName (i))));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      actualParameters->append_expression (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getGlobalToLocalMappingName (i))));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == DIRECT)
    {
      actualParameters->append_expression (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatDeviceName (i))));
    }
  }

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizes)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffs)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMap)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::poffset)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pnelems)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcol)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcol)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::blockOffset)));

  SgExprStatement * callStatement = buildFunctionCallStmt (kernelSubroutineName
      + "<<<" + ROSEHelper::getFirstVariableName (variableDeclarations->get (
      CUDA::Fortran::VariableNames::blocksPerGrid)) + ", "
      + ROSEHelper::getFirstVariableName (variableDeclarations->get (
          CUDA::Fortran::VariableNames::threadsPerBlock)) + ", "
      + ROSEHelper::getFirstVariableName (variableDeclarations->get (
          CUDA::Fortran::VariableNames::sharedMemorySize)) + ">>>",
      buildVoidType (), actualParameters, subroutineScope);

  return callStatement;
}

void
FortranCUDAHostSubroutineIndirectLoop::createConvertCPointersStatements ()
{

}

void
FortranCUDAHostSubroutineIndirectLoop::createExecutionPlanExecutionStatements ()
{
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;
  using std::string;

  SgBasicBlock * loopBody = buildBasicBlock ();

  /*
   * ======================================================
   * Build the statements inside the body of the loop
   * ======================================================
   */

  /*
   * ======================================================
   * Statement to initialise the grid dimension of the
   * CUDA kernel
   * ======================================================
   */

  SgExpression * arrayIndexExpression1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::col)),
      buildIntVal (1));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::ncolblk)),
      arrayIndexExpression1);

  SgExprStatement * statement1 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::VariableNames::blocksPerGrid)),
      arrayExpression1);

  loopBody->append_statement (statement1);

  /*
   * ======================================================
   * Statement to initialise the number of threads per block
   * in the CUDA kernel
   * ======================================================
   */

  SgExprStatement * statement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          CUDA::Fortran::VariableNames::threadsPerBlock)),
      buildOpaqueVarRefExp ("FOP_BLOCK_SIZE", subroutineScope));

  loopBody->append_statement (statement2);

  /*
   * ======================================================
   * Statement to initialise the shared memory size in the
   * CUDA kernel
   * ======================================================
   */

  SgDotExp * dotExpression3 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
      buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::nshared,
          subroutineScope));

  SgExprStatement * statement3 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          CUDA::Fortran::VariableNames::sharedMemorySize)), dotExpression3);

  loopBody->append_statement (statement3);

  /*
   * ======================================================
   * Statement to call the kernel
   * ======================================================
   */

  loopBody->append_statement (createKernelFunctionCallStatement ());

  /*
   * ======================================================
   * Statement to wait for CUDA threads to synchronise
   * ======================================================
   */

  loopBody->append_statement (createThreadSynchroniseCall ());

  /*
   * ======================================================
   * Statement to increment the block offset
   * ======================================================
   */

  SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::blockOffset)),
      buildVarRefExp (variableDeclarations->get (
          CUDA::Fortran::VariableNames::blocksPerGrid)));

  SgStatement * statement4 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::blockOffset)),
      addExpression4);

  loopBody->append_statement (statement4);

  /*
   * ======================================================
   * Add the do-while loop
   * ======================================================
   */

  /*
   * ======================================================
   * The loop starts counting from 0
   * ======================================================
   */

  SgAssignOp * initializationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::col)),
      buildIntVal (0));

  /*
   * ======================================================
   * The loop stops counting at the number of colours in
   * the plan, minus one
   * ======================================================
   */

  SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
      buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::ncolors,
          subroutineScope));

  SgExpression * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFortranDo * fortranDoStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  appendStatement (fortranDoStatement, subroutineScope);
}

void
FortranCUDAHostSubroutineIndirectLoop::createCToFortranPointerConversionStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildGreaterOrEqualOp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildMultiplyOp;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;

  /*
   * ======================================================
   * Variable references needed in the following expressions
   * and statements
   * ======================================================
   */

  SgVarRefExp * actualPlanReference = buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan));

  SgVarRefExp * indsReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::inds));

  SgVarRefExp * indsNumberReference = buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber));

  SgVarRefExp * ncolblkReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::ncolblk));

  SgVarRefExp * opSetFormalArgumentReference = buildVarRefExp (
      variableDeclarations->get (VariableNames::getOpSetName ()));

  SgVarRefExp * pblkMapReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMap));

  SgVarRefExp * pblkMapSizeReference = buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize));

  SgVarRefExp * planRetReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet));

  SgVarRefExp * pmapsReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps));

  SgVarRefExp * pnindirectReference = buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pnindirect));

  SgVarRefExp * pindMapsReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindMaps));

  SgVarRefExp * pindSizesReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizes));

  SgVarRefExp * pindSizesSizeReference = buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize));

  SgVarRefExp * pindOffsReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffs));

  SgVarRefExp * pindOffsSizeReference = buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffsSize));

  SgVarRefExp * pnelemsReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnelems));

  SgVarRefExp * pnelemsSizeReference = buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pnelemsSize));

  SgVarRefExp * pnthrcolReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcol));

  SgVarRefExp * pnthrcolSizeReference = buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcolSize));

  SgVarRefExp * poffsetReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::PlanFunction::VariableNames::poffset));

  SgVarRefExp * poffsetSizeReference = buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::poffsetSize));

  SgVarRefExp * pthrcolReference = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcol));

  SgVarRefExp * pthrcolSizeReference = buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize));

  /*
   * ======================================================
   * Opaque variable references needed in the following
   * expressions and statements
   * ======================================================
   */

  SgVarRefExp * blkmap_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::blkmap,
      subroutineScope);

  SgVarRefExp * ind_maps_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::ind_maps,
      subroutineScope);

  SgVarRefExp * ind_offs_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::ind_offs,
      subroutineScope);

  SgVarRefExp * ind_sizes_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::ind_sizes,
      subroutineScope);

  SgVarRefExp * nblocks_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::nblocks,
      subroutineScope);

  SgVarRefExp * nelems_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::nelems,
      subroutineScope);

  SgVarRefExp * nindirect_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::nindirect,
      subroutineScope);

  SgVarRefExp * nthrcol_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::nthrcol,
      subroutineScope);

  SgVarRefExp * offset_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::offset,
      subroutineScope);

  SgVarRefExp * size_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::size, subroutineScope);

  SgVarRefExp * thrcol_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::thrcol,
      subroutineScope);

  /*
   * ======================================================
   * Shape expressions of Fortran arrays needed in the following
   * expressions and statements
   * ======================================================
   */

  SgExpression
      * indsNumberShapeExpression =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber),
              subroutineScope);

  SgExpression
      * argsNumberShapeExpression =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber),
              subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  SgStatement * callStatement = createCToFortranPointerCallStatement (
      planRetReference, actualPlanReference);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCToFortranPointerCallStatement (buildDotExp (
      actualPlanReference, nindirect_Reference), pnindirectReference,
      indsNumberShapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCToFortranPointerCallStatement (buildDotExp (
      actualPlanReference, ind_maps_Reference), pindMapsReference,
      indsNumberShapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCToFortranPointerCallStatement (buildDotExp (
      actualPlanReference, buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::maps,
          subroutineScope)), pmapsReference, argsNumberShapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCToFortranPointerCallStatement (buildDotExp (
      actualPlanReference, buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::maps,
          subroutineScope)), pmapsReference, argsNumberShapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer calls
   * ======================================================
   */

  unsigned int countIndirectArgs = 1;
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgExpression * indexExpression = buildIntVal (countIndirectArgs++);

      SgExpression * pindMapsArraySubscriptReference = buildPntrArrRefExp (
          pindMapsReference, indexExpression);

      SgExpression * pnindirectArraySubscriptReference = buildPntrArrRefExp (
          pnindirectReference, indexExpression);

      callStatement = createCToFortranPointerCallStatement (
          pindMapsArraySubscriptReference, buildVarRefExp (
              variableDeclarations->get (
                  VariableNames::getLocalToGlobalMappingName (i))),
          pnindirectArraySubscriptReference);

      appendStatement (callStatement, subroutineScope);
    }
  }

  /*
   * ======================================================
   * New c_f_pointer calls
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      /*
       * ======================================================
       * Build the guard of the if statement
       * ======================================================
       */

      SgExpression * indexExpression = buildIntVal (i);

      SgExpression * arrayIndexExpression = buildPntrArrRefExp (indsReference,
          indexExpression);

      SgExpression * ifGuardExpression = buildGreaterOrEqualOp (
          arrayIndexExpression, buildIntVal (0));

      /*
       * ======================================================
       * Build the body of the if statement
       * ======================================================
       */

      /*
       * ======================================================
       * 1st statement
       * ======================================================
       */

      SgExpression * expression1 = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getGlobalToLocalMappingName (i))), buildDotExp (
          opSetFormalArgumentReference, size_Reference));

      SgStatement * statement1 = buildExprStatement (expression1);

      /*
       * ======================================================
       * 2nd statement
       * ======================================================
       */

      indexExpression = buildIntVal (i);

      arrayIndexExpression = buildPntrArrRefExp (pmapsReference,
          indexExpression);

      SgVarRefExp * executionPlanMapReference = buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getGlobalToLocalMappingName (i)));

      SgExpression * shapeExpression =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  VariableNames::getGlobalToLocalMappingName (i)),
              subroutineScope);

      SgStatement * statement2 = createCToFortranPointerCallStatement (
          arrayIndexExpression, executionPlanMapReference, shapeExpression);

      /*
       * ======================================================
       * Add the if statement with a NULL else block
       * ======================================================
       */

      SgBasicBlock * ifBlock = buildBasicBlock (statement1, statement2);

      SgIfStmt
          * ifStatement =
              FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
                  ifGuardExpression, ifBlock);

      appendStatement (ifStatement, subroutineScope);
    }
  }

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCToFortranPointerCallStatement (buildDotExp (
      actualPlanReference, ncolblkReference), ncolblkReference, buildDotExp (
      opSetFormalArgumentReference, size_Reference));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  SgExpression * actualPlan_dot_nblocks_Reference = buildDotExp (
      actualPlanReference, nblocks_Reference);

  SgExpression * assignmentExpression = buildAssignOp (pindSizesSizeReference,
      buildMultiplyOp (actualPlan_dot_nblocks_Reference, indsNumberReference));

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement
      = createCToFortranPointerCallStatement (
          buildDotExp (actualPlanReference, ind_sizes_Reference),
          pindSizesReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize),
              subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (pindOffsSizeReference, buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize)));

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement
      = createCToFortranPointerCallStatement (
          buildDotExp (actualPlanReference, ind_offs_Reference),
          pindOffsReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffsSize),
              subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (pblkMapSizeReference,
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement = createCToFortranPointerCallStatement (buildDotExp (
      actualPlanReference, blkmap_Reference), pblkMapReference,
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize),
          subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (poffsetSizeReference,
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement = createCToFortranPointerCallStatement (buildDotExp (
      actualPlanReference, offset_Reference), poffsetReference,
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::poffsetSize),
          subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (pnelemsSizeReference,
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement = createCToFortranPointerCallStatement (buildDotExp (
      actualPlanReference, nelems_Reference), pnelemsReference,
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::pnelemsSize),
          subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (pnthrcolSizeReference,
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement
      = createCToFortranPointerCallStatement (
          buildDotExp (actualPlanReference, nthrcol_Reference),
          pnthrcolReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcolSize),
              subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (pthrcolSizeReference, buildDotExp (
      opSetFormalArgumentReference, size_Reference));

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement = createCToFortranPointerCallStatement (buildDotExp (
      actualPlanReference, thrcol_Reference), pthrcolReference,
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize),
          subroutineScope));

  appendStatement (callStatement, subroutineScope);
}

void
FortranCUDAHostSubroutineIndirectLoop::initialiseDeviceVariablesSizesVariable ()
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgVarRefExp * dataSizesReferences = buildVarRefExp (
          variableDeclarations->get (
              IndirectAndDirectLoop::Fortran::VariableNames::argsSizes));

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclarationOfIndirectLoop->getFieldDeclarations ()->get (
              VariableNames::getOpDatSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgAssignOp * assignmentExpression = buildAssignOp (
          fieldSelectionExpression, buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatSizeName (i))));

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);
    }
  }

  unsigned int countIndirectArgs = 1;
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgVarRefExp * dataSizesReferences = buildVarRefExp (
          variableDeclarations->get (
              IndirectAndDirectLoop::Fortran::VariableNames::argsSizes));

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclarationOfIndirectLoop->getFieldDeclarations ()->get (
              VariableNames::getLocalToGlobalMappingSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgVarRefExp * pnindirect_Reference = buildVarRefExp (
          variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::pnindirect));

      SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
          pnindirect_Reference, buildIntVal (countIndirectArgs));

      SgAssignOp * assignmentExpression = buildAssignOp (
          fieldSelectionExpression, arrayIndexExpression);

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);

      countIndirectArgs++;
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgVarRefExp * dataSizesReferences = buildVarRefExp (
          variableDeclarations->get (
              IndirectAndDirectLoop::Fortran::VariableNames::argsSizes));

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclarationOfIndirectLoop->getFieldDeclarations ()->get (
              VariableNames::getGlobalToLocalMappingSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgAssignOp * assignmentExpression = buildAssignOp (
          fieldSelectionExpression, buildVarRefExp (variableDeclarations->get (
              VariableNames::getGlobalToLocalMappingSizeName (i))));

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == DIRECT)
    {
      SgVarRefExp * dataSizesReferences = buildVarRefExp (
          variableDeclarations->get (
              IndirectAndDirectLoop::Fortran::VariableNames::argsSizes));

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclarationOfIndirectLoop->getFieldDeclarations ()->get (
              VariableNames::getOpDatSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgAssignOp * assignmentExpression = buildAssignOp (
          fieldSelectionExpression, buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatSizeName (i))));

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);
    }
  }

  vector <string> planFunctionSizeVariables;

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffsSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnelemsSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcolSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::poffsetSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize);

  for (vector <string>::iterator it = planFunctionSizeVariables.begin (); it
      != planFunctionSizeVariables.end (); ++it)
  {
    SgVarRefExp * dataSizesReferences = buildVarRefExp (
        variableDeclarations->get (
            IndirectAndDirectLoop::Fortran::VariableNames::argsSizes));

    SgVarRefExp * fieldReference = buildVarRefExp (
        dataSizesDeclarationOfIndirectLoop->getFieldDeclarations ()->get (*it));

    SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
        fieldReference);

    SgAssignOp * assignmentExpression = buildAssignOp (
        fieldSelectionExpression, buildVarRefExp (variableDeclarations->get (
            *it)));

    appendStatement (buildExprStatement (assignmentExpression), subroutineScope);
  }
}

void
FortranCUDAHostSubroutineIndirectLoop::initialiseVariablesAndConstants ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildVarRefExp;
  using SageInterface::appendStatement;

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::blockOffset)),
      buildIntVal (0));

  appendStatement (assignmentStatement, subroutineScope);

  SgExprListExp * initialseConstantsParameters = buildExprListExp ();

  SgExprStatement * initialseConstantsCall = buildFunctionCallStmt (
      initialiseConstantsSubroutine->getSubroutineName (), buildVoidType (),
      initialseConstantsParameters, subroutineScope);

  appendStatement (initialseConstantsCall, subroutineScope);
}

void
FortranCUDAHostSubroutineIndirectLoop::createExecutionPlanDeclarations ()
{
  using SageBuilder::buildPointerType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Creating execution plan variables", 2);

  /*
   * ======================================================
   * Create the variable which passes the sizes of arguments
   * to the kernel
   * ======================================================
   */

  variableDeclarations->add (
      IndirectAndDirectLoop::Fortran::VariableNames::argsSizes,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::VariableNames::argsSizes,
          dataSizesDeclarationOfIndirectLoop->getType (), subroutineScope, 1,
          DEVICE));

  /*
   * ======================================================
   * Create pointers to the execution plan on the C and
   * Fortran side
   * ======================================================
   */

  SgType * c_ptrType = FortranTypesBuilder::buildNewTypeDeclaration ("c_ptr",
      subroutineScope)->get_type ();

  variableDeclarations->add (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet,
          c_ptrType, subroutineScope));

  SgType * op_planType = FortranTypesBuilder::buildNewTypeDeclaration (
      "op_plan", subroutineScope)->get_type ();

  variableDeclarations->add (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan,
          buildPointerType (op_planType), subroutineScope));

  /*
   * ======================================================
   * Create pointer to the
   * ======================================================
   */

  SgType * c_devptrType = FortranTypesBuilder::buildNewTypeDeclaration (
      "c_devptr", subroutineScope)->get_type ();

  variableDeclarations->add (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindMaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindMaps,
          buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope));

  variableDeclarations->add (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps,
          buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope));

  variableDeclarations->add (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindMapsSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindMapsSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          VariableNames::getLocalToGlobalMappingName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getArray_RankOne (
                  FortranTypesBuilder::getFourByteInteger ()), subroutineScope,
              2, DEVICE, ALLOCATABLE));
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

  fourByteIntegerArrays.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::args);

  fourByteIntegerArrays.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::idxs);

  fourByteIntegerArrays.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::maps);

  fourByteIntegerArrays.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::accesses);

  fourByteIntegerArrays.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::inds);

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
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          VariableNames::getGlobalToLocalMappingName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getArray_RankOne (
                  FortranTypesBuilder::getTwoByteInteger ()), subroutineScope,
              2, DEVICE, ALLOCATABLE));
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
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          VariableNames::getGlobalToLocalMappingSizeName (i);

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

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::iterationCounter);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::blockOffset);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::col);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffsSize);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::poffsetSize);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnelemsSize);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcolSize);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize);

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

  integerPointerVariables.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::ncolblk);

  integerPointerVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnindirect);

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

  deviceIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizes);

  deviceIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffs);

  deviceIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMap);

  deviceIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::poffset);

  deviceIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnelems);

  deviceIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcol);

  deviceIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcol);

  for (vector <string>::iterator it = deviceIntegerArrayVariables.begin (); it
      != deviceIntegerArrayVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ()), subroutineScope,
            2, DEVICE, ALLOCATABLE));
  }
}

void
FortranCUDAHostSubroutineIndirectLoop::createStatements ()
{
  createCUDAKernelPrologueStatements ();

  createPlanFunctionParametersPreparationStatements (parallelLoop,
      subroutineScope, variableDeclarations);

  createPlanFunctionCallStatement (subroutineScope, variableDeclarations);

  createCToFortranPointerConversionStatements ();

  initialiseDeviceVariablesSizesVariable ();

  initialiseVariablesAndConstants ();

  createExecutionPlanExecutionStatements ();

  createCUDAKernelEpilogueStatements ();
}

void
FortranCUDAHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  createDataMarshallingLocalVariableDeclarations ();

  createExecutionPlanDeclarations ();

  createCUDAKernelLocalVariableDeclarations ();
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranCUDAHostSubroutineIndirectLoop::FortranCUDAHostSubroutineIndirectLoop (
    std::string const & subroutineName,
    std::string const & userSubroutineName,
    std::string const & kernelSubroutineName,
    ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
    FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclarationOfIndirectLoop) :
  FortranCUDAHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope),
      initialiseConstantsSubroutine (initialiseConstantsSubroutine),
      dataSizesDeclarationOfIndirectLoop (dataSizesDeclarationOfIndirectLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", 2);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
