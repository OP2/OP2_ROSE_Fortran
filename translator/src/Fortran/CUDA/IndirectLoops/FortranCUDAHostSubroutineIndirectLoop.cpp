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

  loopBody->append_statement (createThreadSynchroniseCallStatement ());

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
FortranCUDAHostSubroutineIndirectLoop::createConvertCPointersStatements ()
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
   * New c_f_pointer call
   * ======================================================
   */

  SgStatement * callStatement1 = createCToFortranPointerCallStatement (
      buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet)),
      buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)));

  appendStatement (callStatement1, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
      buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::nindirect,
          subroutineScope));

  SgExpression
      * shapeExpression2 =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber),
              subroutineScope);

  SgStatement * callStatement2 = createCToFortranPointerCallStatement (
      dotExpression2, buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pnindirect)),
      shapeExpression2);

  appendStatement (callStatement2, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  SgDotExp * dotExpression3 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
      buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::ind_maps,
          subroutineScope));

  SgExpression
      * shapeExpression3 =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber),
              subroutineScope);

  SgStatement * callStatement3 = createCToFortranPointerCallStatement (
      dotExpression3, buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindMaps)),
      shapeExpression3);

  appendStatement (callStatement3, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  SgDotExp * dotExpression4 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
      buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::maps,
          subroutineScope));

  SgExpression
      * shapeExpression4 =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber),
              subroutineScope);

  SgStatement * callStatement4 = createCToFortranPointerCallStatement (
      dotExpression4, buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps)),
      shapeExpression4);

  appendStatement (callStatement4, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  SgDotExp * dotExpression5 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
      buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::maps,
          subroutineScope));

  SgExpression
      * shapeExpression5 =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber),
              subroutineScope);

  SgStatement * callStatement5 = createCToFortranPointerCallStatement (
      dotExpression5, buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps)),
      shapeExpression5);

  appendStatement (callStatement5, subroutineScope);

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

      SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::pindMaps)),
          indexExpression);

      SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::pnindirect)),
          indexExpression);

      SgStatement * callStatement = createCToFortranPointerCallStatement (
          arrayIndexExpression, buildVarRefExp (variableDeclarations->get (
              VariableNames::getLocalToGlobalMappingName (i))),
          arrayIndexExpression2);

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

      SgExpression * arrayIndexExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (
              IndirectLoop::Fortran::HostSubroutine::VariableNames::inds)),
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
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpSetName ())), buildOpaqueVarRefExp (
              IndirectLoop::Fortran::HostSubroutine::FieldNames::size,
              subroutineScope)));

      SgStatement * statement1 = buildExprStatement (expression1);

      /*
       * ======================================================
       * 2nd statement
       * ======================================================
       */

      indexExpression = buildIntVal (i);

      arrayIndexExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps)),
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

  SgDotExp * dotExpression6a = buildDotExp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
      buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::ncolblk)));

  SgDotExp * dotExpression6b = buildDotExp (buildVarRefExp (
      variableDeclarations->get (VariableNames::getOpSetName ())),
      buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::size,
          subroutineScope));

  SgStatement * callStatement6 = createCToFortranPointerCallStatement (
      dotExpression6a, buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::ncolblk)),
      dotExpression6b);

  appendStatement (callStatement6, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  SgExpression * actualPlan_dot_nblocks_Reference = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
      buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::nblocks,
          subroutineScope));

  SgExpression
      * assignmentExpression =
          buildAssignOp (
              buildVarRefExp (
                  variableDeclarations->get (
                      IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize)),
              buildMultiplyOp (
                  actualPlan_dot_nblocks_Reference,
                  buildVarRefExp (
                      variableDeclarations->get (
                          IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber))));

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  SgStatement
      * callStatement7 =
          createCToFortranPointerCallStatement (
              buildDotExp (
                  buildVarRefExp (
                      variableDeclarations->get (
                          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
                  buildOpaqueVarRefExp (
                      IndirectLoop::Fortran::HostSubroutine::FieldNames::ind_sizes,
                      subroutineScope)),
              buildVarRefExp (
                  variableDeclarations->get (
                      IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizes)),
              FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                  variableDeclarations->get (
                      IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize),
                  subroutineScope));

  appendStatement (callStatement7, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffsSize)),
      buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize)));

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  SgStatement
      * callStatement8 =
          createCToFortranPointerCallStatement (
              buildDotExp (
                  buildVarRefExp (
                      variableDeclarations->get (
                          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
                  buildOpaqueVarRefExp (
                      IndirectLoop::Fortran::HostSubroutine::FieldNames::ind_offs,
                      subroutineScope)),
              buildVarRefExp (variableDeclarations->get (
                  IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffs)),
              FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                  variableDeclarations->get (
                      IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffsSize),
                  subroutineScope));

  appendStatement (callStatement8, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize)),
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  SgStatement * callStatement9 = createCToFortranPointerCallStatement (
      buildDotExp (buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
          buildOpaqueVarRefExp (
              IndirectLoop::Fortran::HostSubroutine::FieldNames::blkmap,
              subroutineScope)), buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMap)),
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize),
          subroutineScope));

  appendStatement (callStatement9, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::poffsetSize)),
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  SgStatement * callStatement10 = createCToFortranPointerCallStatement (
      buildDotExp (buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
          buildOpaqueVarRefExp (
              IndirectLoop::Fortran::HostSubroutine::FieldNames::offset,
              subroutineScope)), buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::poffset)),
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::poffsetSize),
          subroutineScope));

  appendStatement (callStatement10, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pnelemsSize)),
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  SgStatement * callStatement11 = createCToFortranPointerCallStatement (
      buildDotExp (buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
          buildOpaqueVarRefExp (
              IndirectLoop::Fortran::HostSubroutine::FieldNames::nelems,
              subroutineScope)), buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pnelems)),
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::pnelemsSize),
          subroutineScope));

  appendStatement (callStatement11, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize)),
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  SgStatement
      * callStatement12 =
          createCToFortranPointerCallStatement (
              buildDotExp (
                  buildVarRefExp (
                      variableDeclarations->get (
                          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
                  buildOpaqueVarRefExp (
                      IndirectLoop::Fortran::HostSubroutine::FieldNames::nthrcol,
                      subroutineScope)),
              buildVarRefExp (variableDeclarations->get (
                  IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcol)),
              FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                  variableDeclarations->get (
                      IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcolSize),
                  subroutineScope));

  appendStatement (callStatement12, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize)),
      buildDotExp (buildVarRefExp (variableDeclarations->get (
          VariableNames::getOpSetName ())), buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::size,
          subroutineScope)));

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  SgStatement * callStatement13 = createCToFortranPointerCallStatement (
      buildDotExp (buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan)),
          buildOpaqueVarRefExp (
              IndirectLoop::Fortran::HostSubroutine::FieldNames::thrcol,
              subroutineScope)), buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcol)),
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize),
          subroutineScope));

  appendStatement (callStatement13, subroutineScope);
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
  createOpDatDimensionInitialisationStatements ();

  createCUDAKernelPrologueStatements ();

  createPlanFunctionParametersPreparationStatements (parallelLoop,
      subroutineScope, variableDeclarations);

  createPlanFunctionCallStatement (subroutineScope, variableDeclarations);

  createConvertCPointersStatements ();

  initialiseDeviceVariablesSizesVariable ();

  initialiseVariablesAndConstants ();

  createExecutionPlanExecutionStatements ();

  createCUDAKernelEpilogueStatements ();
}

void
FortranCUDAHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  createOpDatSizesDeclaration (dataSizesDeclarationOfIndirectLoop->getType ());

  createOpDatDimensionsDeclaration (opDatDimensionsDeclaration->getType ());

  createDataMarshallingLocalVariableDeclarations ();

  createCUDAKernelLocalVariableDeclarations ();

  createExecutionPlanDeclarations ();
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
    FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclarationOfIndirectLoop,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  FortranCUDAHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope,
      opDatDimensionsDeclaration), initialiseConstantsSubroutine (
      initialiseConstantsSubroutine), dataSizesDeclarationOfIndirectLoop (
      dataSizesDeclarationOfIndirectLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", 2);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
