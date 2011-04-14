#include <boost/lexical_cast.hpp>
#include <HostSubroutineOfIndirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <ROSEHelper.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
HostSubroutineOfIndirectLoop::createExecutionPlanExecutionStatements (
    KernelSubroutine & kernelSubroutine, ParallelLoop & parallelLoop)
{
  using SageBuilder::buildVoidType;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;
  using std::string;

  /*
   * ======================================================
   * The loop starts counting from 0
   * ======================================================
   */

  SgExpression * initializationExpression = buildIntVal (0);

  /*
   * ======================================================
   * The loop stops counting at the number of colours in
   * the plan, minus one
   * ======================================================
   */

  SgVarRefExp * ncolors_Reference = buildOpaqueVarRefExp (ncolors_FieldName,
      subroutineScope);

  SgVarRefExp * actualPlanReference = buildVarRefExp (
      localVariables_Others[variableName_actualPlan]);

  SgExpression * upperBoundExpression = buildDotExp (actualPlanReference,
      buildSubtractOp (ncolors_Reference, buildIntVal (1)));

  /*
   * ======================================================
   * The stride of the loop counter is 1
   * ======================================================
   */

  SgExpression * strideExpression = buildIntVal (1);

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

  SgVarRefExp * blocksPerGridReference = buildVarRefExp (
      CUDAVariable_blocksPerGrid);

  SgVarRefExp * colReference = buildVarRefExp (
      localVariables_Others[variableName_col]);

  SgVarRefExp * ncolblkReference = buildVarRefExp (
      localVariables_Others[variableName_ncolblk]);

  SgExpression * arrayIndexExpression = buildAddOp (colReference, buildIntVal (
      1));

  SgExpression * arrayExpression = buildPntrArrRefExp (ncolblkReference,
      arrayIndexExpression);

  SgStatement * statement1 = buildExprStatement (buildAssignOp (
      blocksPerGridReference, arrayExpression));

  /*
   * ======================================================
   * Statement to initialise the number of threads per block
   * in the CUDA kernel
   * ======================================================
   */

  SgVarRefExp * threadsPerBlockReference = buildVarRefExp (
      CUDAVariable_threadsPerBlock);

  string const FOP_BLOCK_SIZE_VariableName = "FOP_BLOCK_SIZE";

  SgVarRefExp * FOP_BLOCK_SIZE_Reference = buildOpaqueVarRefExp (
      FOP_BLOCK_SIZE_VariableName, subroutineScope);

  SgStatement * statement2 = buildExprStatement (buildAssignOp (
      threadsPerBlockReference, FOP_BLOCK_SIZE_Reference));

  /*
   * ======================================================
   * Statement to initialise the shared memory size in the
   * CUDA kernel
   * ======================================================
   */

  SgVarRefExp * sharedMemorySizeReference = buildVarRefExp (
      CUDAVariable_sharedMemorySize);

  SgVarRefExp * nshared_Reference = buildOpaqueVarRefExp (nshared_FieldName,
      subroutineScope);

  SgStatement * statement3 = buildExprStatement (buildAssignOp (
      sharedMemorySizeReference, buildDotExp (actualPlanReference,
          nshared_Reference)));

  /*
   * ======================================================
   * Statement to launch the CUDA kernel
   * ======================================================
   */

  SgExprListExp * kernelParameters = buildExprListExp ();

  SgExprStatement * statement4 = buildFunctionCallStmt (
      kernelSubroutine.getSubroutineName () + "<<<"
          + ROSEHelper::getFirstVariableName (CUDAVariable_blocksPerGrid)
          + ", " + ROSEHelper::getFirstVariableName (
          CUDAVariable_threadsPerBlock) + ", "
          + ROSEHelper::getFirstVariableName (CUDAVariable_sharedMemorySize)
          + ">>>", buildVoidType (), kernelParameters, subroutineScope);

  /*
   * ======================================================
   * Statement to wait for CUDA threads to synchronize
   * ======================================================
   */

  SgFunctionSymbol * cudaThreadSynchronizeFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("cudaThreadSynchronize",
          subroutineScope);

  SgVarRefExp * threadSynchRetReference = buildVarRefExp (
      localVariables_Others[variableName_threadSynchRet]);

  SgFunctionCallExp * threadSynchRetFunctionCall = buildFunctionCallExp (
      cudaThreadSynchronizeFunctionSymbol, buildExprListExp ());

  SgStatement * statement5 = buildExprStatement (buildAssignOp (
      threadSynchRetReference, threadSynchRetFunctionCall));

  /*
   * ======================================================
   * Statement to increment the block offset
   * ======================================================
   */

  SgVarRefExp * blockOffsetReference = buildVarRefExp (
      localVariables_Others[variableName_blockOffset]);

  SgAddOp * incrementExpression = buildAddOp (blockOffsetReference,
      blocksPerGridReference);

  SgStatement * statement6 = buildExprStatement (buildAssignOp (
      blockOffsetReference, incrementExpression));

  /*
   * ======================================================
   * Add the do-while loop
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock (statement1, statement2,
      statement3, statement4, statement5, statement6);

  SgFortranDo * fortranDoStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, strideExpression,
          loopBody);

  appendStatement (fortranDoStatement, subroutineScope);
}

void
HostSubroutineOfIndirectLoop::initialiseVariablesAndConstants (
    InitialiseConstantsSubroutine & initialiseConstantsSubroutine)
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageInterface::appendStatement;

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localVariables_Others[variableName_blockOffset], buildIntVal (0),
      subroutineScope);

  SgExprListExp * initialseConstantsParameters = buildExprListExp ();

  SgExprStatement * initialseConstantsCall = buildFunctionCallStmt (
      initialiseConstantsSubroutine.getSubroutineName (), buildVoidType (),
      initialseConstantsParameters, subroutineScope);

  appendStatement (initialseConstantsCall, subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createPlanCToForttranPointerConversionStatements (
    ParallelLoop & parallelLoop)
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
      localVariables_Others[variableName_actualPlan]);

  SgVarRefExp * indsReference = buildVarRefExp (
      localVariables_Others[variableName_inds]);

  SgVarRefExp * indsNumberReference = buildVarRefExp (
      localVariables_Others[variableName_indsNumber]);

  SgVarRefExp * mapsReference = buildVarRefExp (
      localVariables_Others[variableName_maps]);

  SgVarRefExp * ncolblkReference = buildVarRefExp (
      localVariables_Others[variableName_ncolblk]);

  SgVarRefExp * opSetFormalArgumentReference = buildVarRefExp (
      formalParameter_OP_SET);

  SgVarRefExp * pblkMapReference = buildVarRefExp (
      localVariables_Others[variableName_pblkMap]);

  SgVarRefExp * pblkMapSizeReference = buildVarRefExp (
      localVariables_Others[variableName_pblkMapSize]);

  SgVarRefExp * planRetReference = buildVarRefExp (
      localVariables_Others[variableName_planRet]);

  SgVarRefExp * pmapsReference = buildVarRefExp (
      localVariables_Others[variableName_pmaps]);

  SgVarRefExp * pnindirectReference = buildVarRefExp (
      localVariables_Others[variableName_pnindirect]);

  SgVarRefExp * pindMapsReference = buildVarRefExp (
      localVariables_Others[variableName_pindMaps]);

  SgVarRefExp * pindSizesReference = buildVarRefExp (
      localVariables_Others[variableName_pindSizes]);

  SgVarRefExp * pindSizesSizeReference = buildVarRefExp (
      localVariables_Others[variableName_pindSizesSize]);

  SgVarRefExp * pindOffsReference = buildVarRefExp (
      localVariables_Others[variableName_pindOffs]);

  SgVarRefExp * pindOffsSizeReference = buildVarRefExp (
      localVariables_Others[variableName_pindOffsSize]);

  SgVarRefExp * pnelemsReference = buildVarRefExp (
      localVariables_Others[variableName_pnelems]);

  SgVarRefExp * pnelemsSizeReference = buildVarRefExp (
      localVariables_Others[variableName_pnelemsSize]);

  SgVarRefExp * pnthrcolReference = buildVarRefExp (
      localVariables_Others[variableName_pnthrcol]);

  SgVarRefExp * pnthrcolSizeReference = buildVarRefExp (
      localVariables_Others[variableName_pnthrcolSize]);

  SgVarRefExp * poffsetReference = buildVarRefExp (
      localVariables_Others[variableName_poffset]);

  SgVarRefExp * poffsetSizeReference = buildVarRefExp (
      localVariables_Others[variableName_poffsetSize]);

  SgVarRefExp * pthrcolReference = buildVarRefExp (
      localVariables_Others[variableName_pthrcol]);

  SgVarRefExp * pthrcolSizeReference = buildVarRefExp (
      localVariables_Others[variableName_pthrcolSize]);

  /*
   * ======================================================
   * Opaque variable references needed in the following
   * expressions and statements
   * ======================================================
   */

  SgVarRefExp * blkmap_Reference = buildOpaqueVarRefExp (blkmap_FieldName,
      subroutineScope);

  SgVarRefExp * ind_maps_Reference = buildOpaqueVarRefExp (ind_maps_FieldName,
      subroutineScope);

  SgVarRefExp * ind_offs_Reference = buildOpaqueVarRefExp (ind_offs_FieldName,
      subroutineScope);

  SgVarRefExp * ind_sizes_Reference = buildOpaqueVarRefExp (
      ind_sizes_FieldName, subroutineScope);

  SgVarRefExp * nblocks_Reference = buildOpaqueVarRefExp (nblocks_FieldName,
      subroutineScope);

  SgVarRefExp * nelems_Reference = buildOpaqueVarRefExp (nelems_FieldName,
      subroutineScope);

  SgVarRefExp * nindirect_Reference = buildOpaqueVarRefExp (
      nindirect_FieldName, subroutineScope);

  SgVarRefExp * nthrcol_Reference = buildOpaqueVarRefExp (nthrcol_FieldName,
      subroutineScope);

  SgVarRefExp * offset_Reference = buildOpaqueVarRefExp (offset_FieldName,
      subroutineScope);

  SgVarRefExp * size_Reference = buildOpaqueVarRefExp (size_FieldName,
      subroutineScope);

  SgVarRefExp * thrcol_Reference = buildOpaqueVarRefExp (thrcol_FieldName,
      subroutineScope);

  /*
   * ======================================================
   * Shape expressions of Fortran arrays needed in the following
   * expressions and statements
   * ======================================================
   */

  SgExpression * indsNumberShapeExpression =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[variableName_indsNumber], subroutineScope);

  SgExpression * argsNumberShapeExpression =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[variableName_argsNumber], subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  SgStatement * callStatement = createCallToC_F_POINTER (planRetReference,
      actualPlanReference);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      nindirect_Reference), pnindirectReference, indsNumberShapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      ind_maps_Reference), pindMapsReference, indsNumberShapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      buildOpaqueVarRefExp (maps_FieldName, subroutineScope)), pmapsReference,
      argsNumberShapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      buildOpaqueVarRefExp (maps_FieldName, subroutineScope)), pmapsReference,
      argsNumberShapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer calls
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      SgExpression * indexExpression = buildIntVal (i);

      SgExpression * pindMapsArraySubscriptReference = buildPntrArrRefExp (
          pindMapsReference, indexExpression);

      SgExpression * pnindirectArraySubscriptReference = buildPntrArrRefExp (
          pnindirectReference, indexExpression);

      callStatement = createCallToC_F_POINTER (pindMapsArraySubscriptReference,
          buildVarRefExp (localVariables_ExecutionPlan_IndirectMaps[i]),
          pnindirectArraySubscriptReference);

      appendStatement (callStatement, subroutineScope);
    }
  }

  /*
   * ======================================================
   * New c_f_pointer calls
   * ======================================================
   */

  for (map <unsigned int, SgVariableDeclaration *>::const_iterator it =
      localVariables_ExecutionPlan_OP_MAP_Size.begin (); it
      != localVariables_ExecutionPlan_OP_MAP_Size.end (); ++it)
  {
    /*
     * ======================================================
     * Build the guard of the if statement
     * ======================================================
     */

    SgExpression * indexExpression = buildIntVal (it->first);

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

    SgExpression * expression1 = buildAssignOp (buildVarRefExp (it->second),
        buildDotExp (opSetFormalArgumentReference, size_Reference));

    SgStatement * statement1 = buildExprStatement (expression1);

    /*
     * ======================================================
     * 2nd statement
     * ======================================================
     */

    indexExpression = buildIntVal (it->first);

    arrayIndexExpression = buildPntrArrRefExp (mapsReference, indexExpression);

    SgVarRefExp * executionPlanMapReference = buildVarRefExp (
        localVariables_ExecutionPlan_OP_MAP[it->first]);

    SgExpression * shapeExpression =
        FortranStatementsAndExpressionsBuilder::buildShapeExpression (
            it->second, subroutineScope);

    SgStatement * statement2 = createCallToC_F_POINTER (arrayIndexExpression,
        executionPlanMapReference, shapeExpression);

    /*
     * ======================================================
     * Add the if statement with a NULL else block
     * ======================================================
     */

    SgBasicBlock * ifBlock = buildBasicBlock (statement1, statement2);

    SgIfStmt * ifStatement =
        FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
            ifGuardExpression, ifBlock);

    appendStatement (ifStatement, subroutineScope);
  }

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      ncolblkReference), ncolblkReference, argsNumberShapeExpression);

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

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      ind_sizes_Reference), pindSizesReference,
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[variableName_pindSizesSize], subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (pindOffsSizeReference, buildVarRefExp (
      localVariables_Others[variableName_pindSizesSize]));

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      ind_offs_Reference), pindOffsReference,
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[variableName_pindOffsSize], subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (pblkMapSizeReference,
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      blkmap_Reference), pblkMapReference,
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[variableName_pblkMapSize], subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (poffsetSizeReference,
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      offset_Reference), poffsetReference,
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[variableName_poffsetSize], subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (pnelemsSizeReference,
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      nelems_Reference), pnelemsReference,
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[variableName_pnelemsSize], subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (pnthrcolSizeReference,
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      nthrcol_Reference), pnthrcolReference,
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[variableName_pnthrcolSize], subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (pthrcolSizeReference,
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      thrcol_Reference), pthrcolReference,
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[variableName_pthrcolSize], subroutineScope));

  appendStatement (callStatement, subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createPlanFunctionCallStatement ()
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;
  using std::map;
  using std::make_pair;
  using std::string;

  SgVarRefExp * parameter1 = buildVarRefExp (formalParameter_SubroutineName);

  SgVarRefExp * parameter2 = buildVarRefExp (formalParameter_OP_SET);

  SgVarRefExp * parameter3 = buildVarRefExp (
      localVariables_Others[variableName_argsNumber]);

  SgVarRefExp * parameter4 = buildVarRefExp (
      localVariables_Others[variableName_args]);

  SgVarRefExp * parameter5 = buildVarRefExp (
      localVariables_Others[variableName_idxs]);

  SgVarRefExp * parameter6 = buildVarRefExp (
      localVariables_Others[variableName_maps]);

  SgVarRefExp * parameter7 = buildVarRefExp (
      localVariables_Others[variableName_accesses]);

  SgVarRefExp * parameter8 = buildVarRefExp (
      localVariables_Others[variableName_indsNumber]);

  SgVarRefExp * parameter9 = buildVarRefExp (
      localVariables_Others[variableName_inds]);

  SgExprListExp * cplan_ActualParameters = buildExprListExp (parameter1,
      parameter2, parameter3, parameter4, parameter5, parameter6, parameter7,
      parameter8, parameter9);

  SgFunctionSymbol * cplanFunctionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine ("c_f_pointer",
          subroutineScope);

  SgFunctionCallExp * cplanFunctionCall = buildFunctionCallExp (
      cplanFunctionSymbol, cplan_ActualParameters);

  SgVarRefExp * cplanFunctionReturnReference = buildVarRefExp (
      localVariables_Others[variableName_planRet]);

  SgExpression * assignmentExpression = buildAssignOp (
      cplanFunctionReturnReference, cplanFunctionCall);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createDoLoopToCorrectIndexing (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNotEqualOp;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * The loop starts counting from 1
   * ======================================================
   */
  SgVarRefExp * iterationCounterReference = buildVarRefExp (
      localVariables_Others[variableName_IterationCounter]);

  SgExpression * initializationExpression = buildAssignOp (
      iterationCounterReference, buildIntVal (1));

  /*
   * ======================================================
   * The loop stops counting at the number of OP_DAT argument
   * groups
   * ======================================================
   */
  SgExpression * upperBoundExpression = buildIntVal (
      parallelLoop.getNumberOf_OP_DAT_ArgumentGroups ());

  /*
   * ======================================================
   * The stride of the loop counter is 1
   * ======================================================
   */
  SgExpression * strideExpression = buildIntVal (1);

  /*
   * ======================================================
   * Build the body of the do-loop
   * ======================================================
   */
  SgVarRefExp * opIndirectionArrayReference = buildVarRefExp (
      localVariables_Others[variableName_idxs]);

  SgExpression * arrayIndexExpression = buildPntrArrRefExp (
      opIndirectionArrayReference, iterationCounterReference);

  SgExpression * minusOneExpression = buildSubtractOp (arrayIndexExpression,
      buildIntVal (1));

  SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
      minusOneExpression);

  SgStatement * assignmentStatement = buildExprStatement (assignmentExpression);

  SgBasicBlock * thenBlock = buildBasicBlock (assignmentStatement);

  SgExpression * ifGuardExpression = buildNotEqualOp (arrayIndexExpression,
      buildIntVal (-1));

  SgIfStmt * ifStatement =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, thenBlock);

  SgBasicBlock * loopBodyBlock = buildBasicBlock (ifStatement);

  /*
   * ======================================================
   * Add the do-loop statement
   * ======================================================
   */
  SgFortranDo * fortranDoStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, strideExpression,
          loopBodyBlock);

  appendStatement (fortranDoStatement, subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createExecutionPlanStatements (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opDatFormalArgumentReference = buildVarRefExp (
        formalParameters_OP_DAT[i]);

    SgExpression * indexField = buildDotExp (opDatFormalArgumentReference,
        buildOpaqueVarRefExp ("index", subroutineScope));

    SgVarRefExp * opDatArrayReference = buildVarRefExp (
        localVariables_Others[variableName_args]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opDatArrayReference, indexExpression);

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        indexField);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opIndexFormalArgumentReference = buildVarRefExp (
        formalParameters_OP_INDIRECTION[i]);

    SgVarRefExp * opIndirectionArrayReference = buildVarRefExp (
        localVariables_Others[variableName_idxs]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opIndirectionArrayReference, indexExpression);

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        opIndexFormalArgumentReference);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);
  }

  createDoLoopToCorrectIndexing (parallelLoop);

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opMapFormalArgumentReference = buildVarRefExp (
        formalParameters_OP_MAP[i]);

    SgExpression * indexField = buildDotExp (opMapFormalArgumentReference,
        buildOpaqueVarRefExp ("index", subroutineScope));

    SgVarRefExp * opMapArrayReference = buildVarRefExp (
        localVariables_Others[variableName_maps]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opMapArrayReference, indexExpression);

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        indexField);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opAccessFormalArgumentReference = buildVarRefExp (
        formalParameters_OP_ACCESS[i]);

    SgVarRefExp * opAccessArrayReference = buildVarRefExp (
        localVariables_Others[variableName_accesses]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opAccessArrayReference, indexExpression);

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        opAccessFormalArgumentReference);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * indsArrayReference = buildVarRefExp (
        localVariables_Others[variableName_inds]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        indsArrayReference, indexExpression);

    SgExpression * rhsExpression;

    if (parallelLoop.get_OP_MAP_Value (i) == DIRECT)
    {
      rhsExpression = buildIntVal (-1);
    }
    else
    {
      rhsExpression = buildIntVal (0);
    }

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        rhsExpression);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);
  }

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localVariables_Others[variableName_argsNumber], buildIntVal (
          parallelLoop.getNumberOf_OP_DAT_ArgumentGroups ()), subroutineScope);

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localVariables_Others[variableName_indsNumber], buildIntVal (
          parallelLoop.getNumberOfDistinctIndirect_OP_DAT_Arguments ()),
      subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createExecutionPlanLocalVariables (
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildPointerType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;
  using std::make_pair;
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

  SgVariableDeclaration * variableDeclaration =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName_planRet, c_ptrType, subroutineScope);

  localVariables_Others.insert (make_pair (variableName_planRet,
      variableDeclaration));

  SgType * op_planType = FortranTypesBuilder::buildNewTypeDeclaration (
      "op_plan", subroutineScope)->get_type ();

  SgVariableDeclaration * variableDeclaration_ExecutionPlan_FortranPointer =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName_actualPlan, op_planType, subroutineScope);

  localVariables_Others.insert (make_pair (variableName_actualPlan,
      variableDeclaration_ExecutionPlan_FortranPointer));

  /*
   * ======================================================
   * Create pointer to the
   * ======================================================
   */

  SgType * c_devptrType = FortranTypesBuilder::buildNewTypeDeclaration (
      "c_devptr", subroutineScope)->get_type ();

  variableDeclaration
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName_pindMaps,
          buildPointerType (FortranTypesBuilder::getArray_RankOne (
              c_devptrType, 1,
              parallelLoop.getNumberOfDistinctIndirect_OP_DAT_Arguments ())),
          subroutineScope);

  localVariables_Others.insert (make_pair (variableName_pindMaps,
      variableDeclaration));

  variableDeclaration
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName_pmaps, buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope);

  localVariables_Others.insert (make_pair (variableName_pmaps,
      variableDeclaration));

  variableDeclaration
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName_pindMapsSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  localVariables_Others.insert (make_pair (variableName_pindMapsSize,
      variableDeclaration));

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "pindMaps" + lexical_cast <string> (i);

      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getArray_RankOne (
                  FortranTypesBuilder::getFourByteInteger ()), subroutineScope);

      variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
      variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();

      localVariables_ExecutionPlan_IndirectMaps.insert (make_pair (i,
          variableDeclaration));
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "pindMaps" + lexical_cast <string> (i)
          + "Size";

      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);

      localVariables_ExecutionPlan_IndirectMaps_Size.insert (make_pair (i,
          variableDeclaration));
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
  fourByteIntegerArrays.push_back (variableName_args);
  fourByteIntegerArrays.push_back (variableName_idxs);
  fourByteIntegerArrays.push_back (variableName_maps);
  fourByteIntegerArrays.push_back (variableName_accesses);
  fourByteIntegerArrays.push_back (variableName_inds);

  for (vector <string>::iterator it = fourByteIntegerArrays.begin (); it
      != fourByteIntegerArrays.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger (), 1,
                parallelLoop.getNumberOf_OP_DAT_ArgumentGroups ()),
            subroutineScope);

    localVariables_Others.insert (make_pair (*it, variableDeclaration));
  }

  /*
   * ======================================================
   * Create device variables for each distinct OP_DAT
   * (which point to the actual data)
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      string const variableName = "pArg" + lexical_cast <string> (i) + "DatD";

      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, parallelLoop.get_OP_DAT_Type (i), subroutineScope);

      variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
      variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();

      localVariables_ExecutionPlan_OP_DAT.insert (make_pair (i,
          variableDeclaration));
    }
  }

  /*
   * ======================================================
   * Create device integer variables for each distinct OP_DAT
   * which record the size of the array of the actual data
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      string const variableName = "pArg" + lexical_cast <string> (i)
          + "DatDSize";

      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);

      localVariables_ExecutionPlan_OP_DAT_Size.insert (make_pair (i,
          variableDeclaration));
    }
  }

  /*
   * ======================================================
   * Create device integer variables for each OP_DAT
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    string const variableName = "pMaps" + lexical_cast <string> (i);

    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getTwoByteInteger ()), subroutineScope);

    variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
    variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();

    localVariables_ExecutionPlan_OP_MAP.insert (make_pair (i,
        variableDeclaration));
  }

  /*
   * ======================================================
   * Create integer variables
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    string const variableName = "pMaps" + lexical_cast <string> (i) + "Size";

    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, FortranTypesBuilder::getFourByteInteger (),
            subroutineScope);

    localVariables_ExecutionPlan_OP_MAP_Size.insert (make_pair (i,
        variableDeclaration));
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

  fourByteIntegerVariables.push_back (variableName_argsNumber);
  fourByteIntegerVariables.push_back (variableName_indsNumber);
  fourByteIntegerVariables.push_back (variableName_IterationCounter);
  fourByteIntegerVariables.push_back (variableName_blockOffset);
  fourByteIntegerVariables.push_back (variableName_col);
  fourByteIntegerVariables.push_back (variableName_threadSynchRet);
  fourByteIntegerVariables.push_back (variableName_pindSizesSize);
  fourByteIntegerVariables.push_back (variableName_pindOffsSize);
  fourByteIntegerVariables.push_back (variableName_pblkMapSize);
  fourByteIntegerVariables.push_back (variableName_poffsetSize);
  fourByteIntegerVariables.push_back (variableName_pnelemsSize);
  fourByteIntegerVariables.push_back (variableName_pnthrcolSize);
  fourByteIntegerVariables.push_back (variableName_pthrcolSize);

  for (vector <string>::iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildPointerType (FortranTypesBuilder::getFourByteInteger ()),
            subroutineScope);

    localVariables_Others.insert (make_pair (*it, variableDeclaration));
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

  integerPointerVariables.push_back (variableName_ncolblk);
  integerPointerVariables.push_back (variableName_pnindirect);

  for (vector <string>::iterator it = integerPointerVariables.begin (); it
      != integerPointerVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildPointerType (FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ())), subroutineScope);

    localVariables_Others.insert (make_pair (*it, variableDeclaration));
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

  deviceIntegerArrayVariables.push_back (variableName_pindSizes);
  deviceIntegerArrayVariables.push_back (variableName_pindOffs);
  deviceIntegerArrayVariables.push_back (variableName_pblkMap);
  deviceIntegerArrayVariables.push_back (variableName_poffset);
  deviceIntegerArrayVariables.push_back (variableName_pnelems);
  deviceIntegerArrayVariables.push_back (variableName_pnthrcol);
  deviceIntegerArrayVariables.push_back (variableName_pthrcol);

  for (vector <string>::iterator it = deviceIntegerArrayVariables.begin (); it
      != deviceIntegerArrayVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildPointerType (FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ())), subroutineScope);

    variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
    variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();

    localVariables_Others.insert (make_pair (*it, variableDeclaration));
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

HostSubroutineOfIndirectLoop::HostSubroutineOfIndirectLoop (
    std::string const & subroutineName,
    UserDeviceSubroutine & userDeviceSubroutine,
    KernelSubroutine & kernelSubroutine,
    InitialiseConstantsSubroutine & initialiseConstantsSubroutine,
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
    ParallelLoop & parallelLoop, SgScopeStatement * moduleScope) :
  HostSubroutine (subroutineName, userDeviceSubroutine, parallelLoop,
      moduleScope)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", 2);

  createFormalParameters (userDeviceSubroutine, parallelLoop);

  createDataMarshallingLocalVariables (parallelLoop);

  createExecutionPlanLocalVariables (parallelLoop);

  createCUDAKernelVariables ();

  initialiseDataMarshallingLocalVariables (parallelLoop);

  createExecutionPlanStatements (parallelLoop);

  createPlanFunctionCallStatement ();

  createPlanCToForttranPointerConversionStatements (parallelLoop);

  initialiseVariablesAndConstants (initialiseConstantsSubroutine);

  createExecutionPlanExecutionStatements (kernelSubroutine, parallelLoop);

  copyDataBackFromDeviceAndDeallocate (parallelLoop);
}
