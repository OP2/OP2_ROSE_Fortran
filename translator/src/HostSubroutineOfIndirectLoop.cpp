#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <HostSubroutineOfIndirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <ROSEHelper.h>

namespace VariableNames
{
  /*
   * ======================================================
   * Following are names of local variables needed in the host
   * subroutine of an indirect loop
   * ======================================================
   */

  std::string const accesses = "accesses";
  std::string const actualPlan = "actualPlan";
  std::string const args = "args";
  std::string const argsNumber = "argsNumber";
  std::string const blockOffset = "blockOffset";
  std::string const col = "col";
  std::string const IterationCounter = "i";
  std::string const idxs = "idxs";
  std::string const inds = "inds";
  std::string const indsNumber = "indsNumber";
  std::string const maps = "maps";
  std::string const ncolblk = "ncolblk";
  std::string const planRet = "planRet";
}

namespace FieldNames
{
  /*
   * ======================================================
   * Following are field names inside structs variables needed
   * in the host subroutine of an indirect loop. Usually
   * these are opaque to ROSE
   * ======================================================
   */

  std::string const blkmap = "blkmap";
  std::string const ind_maps = "ind_maps";
  std::string const ind_offs = "ind_offs";
  std::string const ind_sizes = "ind_sizes";
  std::string const maps = "maps";
  std::string const nblocks = "nblocks";
  std::string const ncolblk = "ncolblk";
  std::string const ncolors = "ncolors";
  std::string const nelems = "nelems";
  std::string const nindirect = "nindirect";
  std::string const nshared = "nshared";
  std::string const nthrcol = "nthrcol";
  std::string const offset = "offset";
  std::string const size = "size";
  std::string const thrcol = "thrcol";
}

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
HostSubroutineOfIndirectLoop::initialiseDeviceVariablesSizesVariable (
    ParallelLoop & parallelLoop,
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration)
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

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * dataSizesReferences = buildVarRefExp (
          localVariables_Others[OtherVariableNames::argsSizes]);

      SgVarRefExp * fieldReference = buildVarRefExp (
          deviceDataSizesDeclaration.get_OP_DAT_SizeFieldDeclaration (i));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgAssignOp * assignmentExpression = buildAssignOp (
          fieldSelectionExpression, buildVarRefExp (
              localVariables_ExecutionPlan_OP_DAT_Size[i]));

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * dataSizesReferences = buildVarRefExp (
          localVariables_Others[OtherVariableNames::argsSizes]);

      SgVarRefExp
          * fieldReference =
              buildVarRefExp (
                  deviceDataSizesDeclaration.get_LocalToGlobalMappingSizeFieldDeclaration (
                      i));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgVarRefExp
          * pnindirect_Reference =
              buildVarRefExp (
                  localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pnindirect]);

      SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
          pnindirect_Reference, buildIntVal (i));

      SgAssignOp * assignmentExpression = buildAssignOp (
          fieldSelectionExpression, arrayIndexExpression);

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * dataSizesReferences = buildVarRefExp (
          localVariables_Others[OtherVariableNames::argsSizes]);

      SgVarRefExp
          * fieldReference =
              buildVarRefExp (
                  deviceDataSizesDeclaration.get_GlobalToLocalMappingSizeFieldDeclaration (
                      i));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgAssignOp * assignmentExpression = buildAssignOp (
          fieldSelectionExpression, buildVarRefExp (
              localVariables_ExecutionPlan_OP_MAP_Size[i]));

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == DIRECT)
    {
      SgVarRefExp * dataSizesReferences = buildVarRefExp (
          localVariables_Others[OtherVariableNames::argsSizes]);

      SgVarRefExp * fieldReference = buildVarRefExp (
          deviceDataSizesDeclaration.get_OP_DAT_SizeFieldDeclaration (i));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgAssignOp * assignmentExpression = buildAssignOp (
          fieldSelectionExpression, buildVarRefExp (
              localVariables_ExecutionPlan_OP_DAT_Size[i]));

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);
    }
  }

  vector <string> planFunctionSizeVariables;
<<<<<<< HEAD
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pblkMapSize);
//  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pindMapsSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pindOffsSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pindSizesSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pnelemsSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pnthrcolSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::poffsetSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pthrcolSize);
=======

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindMapsSize);

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
>>>>>>> d54656112ee65584301e2d7380ea8ffaca641891

  for (vector <string>::iterator it = planFunctionSizeVariables.begin (); it
      != planFunctionSizeVariables.end (); ++it)
  {
    SgVarRefExp * dataSizesReferences = buildVarRefExp (
        localVariables_Others[OtherVariableNames::argsSizes]);

    SgVarRefExp * fieldReference = buildVarRefExp (
        deviceDataSizesDeclaration.getPlanVariableSizeFieldDeclaration (*it));

    SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
        fieldReference);

    SgAssignOp * assignmentExpression = buildAssignOp (
        fieldSelectionExpression, buildVarRefExp (localVariables_Others[*it]));

    appendStatement (buildExprStatement (assignmentExpression), subroutineScope);
  }
}

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

  SgVarRefExp * colReference = buildVarRefExp (
      localVariables_Others[VariableNames::col]);

  SgExpression * initializationExpression = buildAssignOp (colReference,
      buildIntVal (0));

  /*
   * ======================================================
   * The loop stops counting at the number of colours in
   * the plan, minus one
   * ======================================================
   */

  SgVarRefExp * ncolors_Reference = buildOpaqueVarRefExp (FieldNames::ncolors,
      subroutineScope);

  SgVarRefExp * actualPlanReference = buildVarRefExp (
      localVariables_Others[VariableNames::actualPlan]);

  SgExpression * upperBoundExpression = buildSubtractOp (buildDotExp (
      actualPlanReference, ncolors_Reference), buildIntVal (1));

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

  SgVarRefExp * ncolblkReference = buildVarRefExp (
      localVariables_Others[VariableNames::ncolblk]);

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

  SgVarRefExp * nshared_Reference = buildOpaqueVarRefExp (FieldNames::nshared,
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

  SgVarRefExp * argsSizesReference = buildVarRefExp (
      localVariables_Others[OtherVariableNames::argsSizes]);

  kernelParameters->append_expression (argsSizesReference);

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * OP_DAT_argumentReference = buildVarRefExp (
          localVariables_OP_DAT_VariablesOnDevice[i]);

      kernelParameters->append_expression (OP_DAT_argumentReference);

      SgVarRefExp * IndirectMap_argumentReference = buildVarRefExp (
          localVariables_ExecutionPlan_IndirectMaps[i]);

      kernelParameters->append_expression (IndirectMap_argumentReference);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * OP_MAP_argumentReference = buildVarRefExp (
          localVariables_ExecutionPlan_OP_MAP[i]);

      kernelParameters->append_expression (OP_MAP_argumentReference);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == DIRECT)
    {
      SgVarRefExp * OP_DAT_argumentReference = buildVarRefExp (
          localVariables_OP_DAT_VariablesOnDevice[i]);

      kernelParameters->append_expression (OP_DAT_argumentReference);
    }
  }

  SgVarRefExp
      * pindSizesReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizes]);

  kernelParameters->append_expression (pindSizesReference);

  SgVarRefExp
      * pindOffsReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffs]);

  kernelParameters->append_expression (pindOffsReference);

  SgVarRefExp
      * pblkMapReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMap]);

  kernelParameters->append_expression (pblkMapReference);

  SgVarRefExp
      * poffsetReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::poffset]);

  kernelParameters->append_expression (poffsetReference);

  SgVarRefExp
      * pnelemsReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pnelems]);

  kernelParameters->append_expression (pnelemsReference);

  SgVarRefExp
      * pnthrcolReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcol]);

  kernelParameters->append_expression (pnthrcolReference);

  SgVarRefExp
      * pthrcolReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcol]);

  kernelParameters->append_expression (pthrcolReference);

  SgVarRefExp * blockOffsetReference = buildVarRefExp (
      localVariables_Others[VariableNames::blockOffset]);

  kernelParameters->append_expression (blockOffsetReference);

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

  SgStatement * statement5 = buildThreadSynchroniseFunctionCall (
      subroutineScope);

  /*
   * ======================================================
   * Statement to increment the block offset
   * ======================================================
   */

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
      localVariables_Others[VariableNames::blockOffset], buildIntVal (0),
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
      localVariables_Others[VariableNames::actualPlan]);

  SgVarRefExp * indsReference = buildVarRefExp (
      localVariables_Others[VariableNames::inds]);

  SgVarRefExp * indsNumberReference = buildVarRefExp (
      localVariables_Others[VariableNames::indsNumber]);

  SgVarRefExp * mapsReference = buildVarRefExp (
      localVariables_Others[VariableNames::maps]);

  SgVarRefExp * ncolblkReference = buildVarRefExp (
      localVariables_Others[VariableNames::ncolblk]);

  SgVarRefExp * opSetFormalArgumentReference = buildVarRefExp (
      formalParameter_OP_SET);

  SgVarRefExp
      * pblkMapReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMap]);

  SgVarRefExp
      * pblkMapSizeReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize]);

  SgVarRefExp * planRetReference = buildVarRefExp (
      localVariables_Others[VariableNames::planRet]);

  SgVarRefExp
      * pmapsReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps]);

  SgVarRefExp
      * pnindirectReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pnindirect]);

  SgVarRefExp
      * pindMapsReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindMaps]);

  SgVarRefExp
      * pindSizesReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizes]);

  SgVarRefExp
      * pindSizesSizeReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize]);

  SgVarRefExp
      * pindOffsReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffs]);

  SgVarRefExp
      * pindOffsSizeReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffsSize]);

  SgVarRefExp
      * pnelemsReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pnelems]);

  SgVarRefExp
      * pnelemsSizeReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pnelemsSize]);

  SgVarRefExp
      * pnthrcolReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcol]);

  SgVarRefExp
      * pnthrcolSizeReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcolSize]);

  SgVarRefExp
      * poffsetReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::poffset]);

  SgVarRefExp
      * poffsetSizeReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::poffsetSize]);

  SgVarRefExp
      * pthrcolReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcol]);

  SgVarRefExp
      * pthrcolSizeReference =
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize]);

  /*
   * ======================================================
   * Opaque variable references needed in the following
   * expressions and statements
   * ======================================================
   */

  SgVarRefExp * blkmap_Reference = buildOpaqueVarRefExp (FieldNames::blkmap,
      subroutineScope);

  SgVarRefExp * ind_maps_Reference = buildOpaqueVarRefExp (
      FieldNames::ind_maps, subroutineScope);

  SgVarRefExp * ind_offs_Reference = buildOpaqueVarRefExp (
      FieldNames::ind_offs, subroutineScope);

  SgVarRefExp * ind_sizes_Reference = buildOpaqueVarRefExp (
      FieldNames::ind_sizes, subroutineScope);

  SgVarRefExp * nblocks_Reference = buildOpaqueVarRefExp (FieldNames::nblocks,
      subroutineScope);

  SgVarRefExp * nelems_Reference = buildOpaqueVarRefExp (FieldNames::nelems,
      subroutineScope);

  SgVarRefExp * nindirect_Reference = buildOpaqueVarRefExp (
      FieldNames::nindirect, subroutineScope);

  SgVarRefExp * nthrcol_Reference = buildOpaqueVarRefExp (FieldNames::nthrcol,
      subroutineScope);

  SgVarRefExp * offset_Reference = buildOpaqueVarRefExp (FieldNames::offset,
      subroutineScope);

  SgVarRefExp * size_Reference = buildOpaqueVarRefExp (FieldNames::size,
      subroutineScope);

  SgVarRefExp * thrcol_Reference = buildOpaqueVarRefExp (FieldNames::thrcol,
      subroutineScope);

  /*
   * ======================================================
   * Shape expressions of Fortran arrays needed in the following
   * expressions and statements
   * ======================================================
   */

  SgExpression * indsNumberShapeExpression =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[VariableNames::indsNumber], subroutineScope);

  SgExpression * argsNumberShapeExpression =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[VariableNames::argsNumber], subroutineScope);

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
      buildOpaqueVarRefExp (FieldNames::maps, subroutineScope)),
      pmapsReference, argsNumberShapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      buildOpaqueVarRefExp (FieldNames::maps, subroutineScope)),
      pmapsReference, argsNumberShapeExpression);

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

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
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
          localVariables_ExecutionPlan_OP_MAP_Size[i]), buildDotExp (
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
          localVariables_ExecutionPlan_OP_MAP[i]);

      SgExpression * shapeExpression =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariables_ExecutionPlan_OP_MAP_Size[i], subroutineScope);

      SgStatement * statement2 = createCallToC_F_POINTER (arrayIndexExpression,
          executionPlanMapReference, shapeExpression);

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

  callStatement
      = createCallToC_F_POINTER (
          buildDotExp (actualPlanReference, ind_sizes_Reference),
          pindSizesReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize],
              subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression
      = buildAssignOp (
          pindOffsSizeReference,
          buildVarRefExp (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize]));

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement
      = createCallToC_F_POINTER (
          buildDotExp (actualPlanReference, ind_offs_Reference),
          pindOffsReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffsSize],
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

  callStatement
      = createCallToC_F_POINTER (
          buildDotExp (actualPlanReference, blkmap_Reference),
          pblkMapReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize],
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

  callStatement
      = createCallToC_F_POINTER (
          buildDotExp (actualPlanReference, offset_Reference),
          poffsetReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::poffsetSize],
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

  callStatement
      = createCallToC_F_POINTER (
          buildDotExp (actualPlanReference, nelems_Reference),
          pnelemsReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pnelemsSize],
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
      = createCallToC_F_POINTER (
          buildDotExp (actualPlanReference, nthrcol_Reference),
          pnthrcolReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcolSize],
              subroutineScope));

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  assignmentExpression = buildAssignOp (pthrcolSizeReference,
      actualPlan_dot_nblocks_Reference);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement
      = createCallToC_F_POINTER (
          buildDotExp (actualPlanReference, thrcol_Reference),
          pthrcolReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize],
              subroutineScope));

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
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;

  SgVarRefExp * parameter1 = buildVarRefExp (formalParameter_SubroutineName);

  SgVarRefExp * indexField = buildOpaqueVarRefExp ("index", subroutineScope);

  SgExpression * parameter2 = buildDotExp (buildVarRefExp (
      formalParameter_OP_SET), indexField);

  SgVarRefExp * parameter3 = buildVarRefExp (
      localVariables_Others[VariableNames::argsNumber]);

  SgVarRefExp * parameter4 = buildVarRefExp (
      localVariables_Others[VariableNames::args]);

  SgVarRefExp * parameter5 = buildVarRefExp (
      localVariables_Others[VariableNames::idxs]);

  SgVarRefExp * parameter6 = buildVarRefExp (
      localVariables_Others[VariableNames::maps]);

  SgVarRefExp * parameter7 = buildVarRefExp (
      localVariables_Others[VariableNames::accesses]);

  SgVarRefExp * parameter8 = buildVarRefExp (
      localVariables_Others[VariableNames::indsNumber]);

  SgVarRefExp * parameter9 = buildVarRefExp (
      localVariables_Others[VariableNames::inds]);

  SgExprListExp * cplan_ActualParameters = buildExprListExp (parameter1,
      parameter2, parameter3, parameter4, parameter5, parameter6, parameter7,
      parameter8, parameter9);

  SgFunctionSymbol * cplanFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("cplan", subroutineScope);

  SgFunctionCallExp * cplanFunctionCall = buildFunctionCallExp (
      cplanFunctionSymbol, cplan_ActualParameters);

  SgVarRefExp * cplanFunctionReturnReference = buildVarRefExp (
      localVariables_Others[VariableNames::planRet]);

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
      localVariables_Others[VariableNames::IterationCounter]);

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
      localVariables_Others[VariableNames::idxs]);

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
        localVariables_Others[VariableNames::args]);

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
        localVariables_Others[VariableNames::idxs]);

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
        localVariables_Others[VariableNames::maps]);

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
        localVariables_Others[VariableNames::accesses]);

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
        localVariables_Others[VariableNames::inds]);

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
      localVariables_Others[VariableNames::argsNumber], buildIntVal (
          parallelLoop.getNumberOf_OP_DAT_ArgumentGroups ()), subroutineScope);

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localVariables_Others[VariableNames::indsNumber], buildIntVal (
          parallelLoop.getNumberOfDistinctIndirect_OP_DAT_Arguments ()),
      subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createExecutionPlanLocalVariables (
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
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

  SgVariableDeclaration * variableDeclaration2 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OtherVariableNames::argsSizes, deviceDataSizesDeclaration.getType (),
          subroutineScope);

  variableDeclaration2->get_declarationModifier ().get_typeModifier ().setDevice ();

  localVariables_Others[OtherVariableNames::argsSizes] = variableDeclaration2;

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
          VariableNames::planRet, c_ptrType, subroutineScope);

  localVariables_Others[VariableNames::planRet] = variableDeclaration;

  SgType * op_planType = FortranTypesBuilder::buildNewTypeDeclaration (
      "op_plan", subroutineScope)->get_type ();

  SgVariableDeclaration * variableDeclaration_ExecutionPlan_FortranPointer =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          VariableNames::actualPlan, buildPointerType (op_planType),
          subroutineScope);

  localVariables_Others[VariableNames::actualPlan]
      = variableDeclaration_ExecutionPlan_FortranPointer;

  /*
   * ======================================================
   * Create pointer to the
   * ======================================================
   */

  SgType * c_devptrType = FortranTypesBuilder::buildNewTypeDeclaration (
      "c_devptr", subroutineScope)->get_type ();

  variableDeclaration
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindMaps,
          buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope);

  //
  //  variableDeclaration
  //      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
  //          PlanFunctionVariables::pindMaps,
  //          buildPointerType (FortranTypesBuilder::getArray_RankOne (
  //              c_devptrType, 1,
  //              parallelLoop.getNumberOfDistinctIndirect_OP_DAT_Arguments ())),
  //          subroutineScope);

  localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindMaps]
      = variableDeclaration;

  variableDeclaration
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps,
          buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope);

  localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps]
      = variableDeclaration;

  variableDeclaration
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindMapsSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  localVariables_Others[IndirectLoop::Fortran::PlanFunction::VariableNames::pindMapsSize]
      = variableDeclaration;

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

      localVariables_ExecutionPlan_IndirectMaps[i] = variableDeclaration;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "pindMaps" + lexical_cast <string> (i)
          + VariableSuffixes::Size;

      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);

      localVariables_ExecutionPlan_IndirectMaps_Size[i] = variableDeclaration;
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
  fourByteIntegerArrays.push_back (VariableNames::args);
  fourByteIntegerArrays.push_back (VariableNames::idxs);
  fourByteIntegerArrays.push_back (VariableNames::maps);
  fourByteIntegerArrays.push_back (VariableNames::accesses);
  fourByteIntegerArrays.push_back (VariableNames::inds);

  for (vector <string>::iterator it = fourByteIntegerArrays.begin (); it
      != fourByteIntegerArrays.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger (), 1,
                parallelLoop.getNumberOf_OP_DAT_ArgumentGroups ()),
            subroutineScope);

    localVariables_Others[*it] = variableDeclaration;
  }

  /*
   * ======================================================
   * Create device variables for each distinct OP_DAT
   * (which point to the actual data)
   * ======================================================
   */

  // DUPLICATED CODE FROM HostSubroutine.cpp
  //  for (unsigned int i = 1; i
  //      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  //  {
  //    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
  //    {
  //      string const variableName = kernelDatArgumentsNames::argNamePrefix + lexical_cast <string> (i);
  //
  //      SgVariableDeclaration * variableDeclaration =
  //          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
  //              variableName, parallelLoop.get_OP_DAT_Type (i), subroutineScope);
  //
  //      variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
  //      variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();
  //
  //      localVariables_ExecutionPlan_OP_DAT[i] = variableDeclaration;
  //    }
  //  }

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
      string const variableName = VariablePrefixes::OP_DAT_Name + lexical_cast <
          string> (i) + VariableSuffixes::Size;

      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);

      localVariables_ExecutionPlan_OP_DAT_Size[i] = variableDeclaration;
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
    if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "pMaps" + lexical_cast <string> (i);

      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getArray_RankOne (
                  FortranTypesBuilder::getTwoByteInteger ()), subroutineScope);

      variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
      variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();

      localVariables_ExecutionPlan_OP_MAP[i] = variableDeclaration;
    }
  }

  /*
   * ======================================================
   * Create integer variables
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "pMaps" + lexical_cast <string> (i)
          + VariableSuffixes::Size;

      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);

      localVariables_ExecutionPlan_OP_MAP_Size[i] = variableDeclaration;
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

  fourByteIntegerVariables.push_back (VariableNames::argsNumber);
  fourByteIntegerVariables.push_back (VariableNames::indsNumber);
  fourByteIntegerVariables.push_back (VariableNames::IterationCounter);
  fourByteIntegerVariables.push_back (VariableNames::blockOffset);
  fourByteIntegerVariables.push_back (VariableNames::col);

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

  /*
   * ======================================================
   * threadSynchRet is used by both direct and indirect
   * loops, then it is now moved to the
   * createCUDAKernelVariables routine
   * ======================================================
   */
  // fourByteIntegerVariables.push_back (threadSynchRet);


  for (vector <string>::iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    //buildPointerType ()
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope);

    localVariables_Others[*it] = variableDeclaration;
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

  integerPointerVariables.push_back (VariableNames::ncolblk);
  integerPointerVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnindirect);

  for (vector <string>::iterator it = integerPointerVariables.begin (); it
      != integerPointerVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildPointerType (FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ())), subroutineScope);

    localVariables_Others[*it] = variableDeclaration;
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
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ()), subroutineScope);

    variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
    variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();

    localVariables_Others[*it] = variableDeclaration;
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

  createExecutionPlanLocalVariables (deviceDataSizesDeclaration, parallelLoop);

  createCUDAKernelVariables ();

  initialiseDataMarshallingLocalVariables (parallelLoop);

  createExecutionPlanStatements (parallelLoop);

  createPlanFunctionCallStatement ();

  createPlanCToForttranPointerConversionStatements (parallelLoop);

  initialiseDeviceVariablesSizesVariable (parallelLoop,
      deviceDataSizesDeclaration);

  initialiseVariablesAndConstants (initialiseConstantsSubroutine);

  createExecutionPlanExecutionStatements (kernelSubroutine, parallelLoop);

  copyDataBackFromDeviceAndDeallocate (parallelLoop);
}
