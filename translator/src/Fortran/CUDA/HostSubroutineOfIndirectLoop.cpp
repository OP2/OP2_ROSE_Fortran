#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <HostSubroutineOfIndirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <ROSEHelper.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

std::string
HostSubroutineOfIndirectLoop::getLocalToGlobalMappingVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectLoop::Fortran::VariablePrefixes::pindMaps + lexical_cast <
      string> (OP_DAT_ArgumentGroup);
}

std::string
HostSubroutineOfIndirectLoop::getGlobalToLocalMappingVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectLoop::Fortran::VariablePrefixes::pMaps
      + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
HostSubroutineOfIndirectLoop::getGlobalToLocalMappingSizeVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectLoop::Fortran::VariablePrefixes::pMaps
      + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + IndirectAndDirectLoop::Fortran::VariableSuffixes::Size;
}

void
HostSubroutineOfIndirectLoop::initialiseDeviceVariablesSizesVariable ()
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
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp
          * dataSizesReferences =
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]);

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclarationOfIndirectLoop->getFieldDeclaration (
              dataSizesDeclarationOfIndirectLoop->get_OP_DAT_SizeFieldName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgAssignOp * assignmentExpression = buildAssignOp (
          fieldSelectionExpression, buildVarRefExp (
              localVariableDeclarations[get_OP_DAT_SizeVariableName (i)]));

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);
    }
  }

  unsigned int countIndirectArgs = 1;
  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp
          * dataSizesReferences =
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]);

      SgVarRefExp
          * fieldReference =
              buildVarRefExp (
                  dataSizesDeclarationOfIndirectLoop->getFieldDeclaration (
                      dataSizesDeclarationOfIndirectLoop->getLocalToGlobalRenumberingSizeFieldName (
                          i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgVarRefExp
          * pnindirect_Reference =
              buildVarRefExp (
                  localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pnindirect]);

      SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
          pnindirect_Reference, buildIntVal (countIndirectArgs));

      SgAssignOp * assignmentExpression = buildAssignOp (
          fieldSelectionExpression, arrayIndexExpression);

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);

      countIndirectArgs++;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp
          * dataSizesReferences =
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]);

      SgVarRefExp
          * fieldReference =
              buildVarRefExp (
                  dataSizesDeclarationOfIndirectLoop->getFieldDeclaration (
                      dataSizesDeclarationOfIndirectLoop->getGlobalToLocalRenumberingSizeFieldName (
                          i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgAssignOp
          * assignmentExpression =
              buildAssignOp (
                  fieldSelectionExpression,
                  buildVarRefExp (
                      localVariableDeclarations[getGlobalToLocalMappingSizeVariableName (
                          i)]));

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == DIRECT)
    {
      SgVarRefExp
          * dataSizesReferences =
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]);

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclarationOfIndirectLoop->getFieldDeclaration (
              dataSizesDeclarationOfIndirectLoop->get_OP_DAT_SizeFieldName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgAssignOp * assignmentExpression = buildAssignOp (
          fieldSelectionExpression, buildVarRefExp (
              localVariableDeclarations[get_OP_DAT_SizeVariableName (i)]));

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
    SgVarRefExp
        * dataSizesReferences =
            buildVarRefExp (
                localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]);

    SgVarRefExp * fieldReference = buildVarRefExp (
        dataSizesDeclarationOfIndirectLoop->getFieldDeclaration (*it));

    SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
        fieldReference);

    SgAssignOp * assignmentExpression = buildAssignOp (
        fieldSelectionExpression, buildVarRefExp (
            localVariableDeclarations[*it]));

    appendStatement (buildExprStatement (assignmentExpression), subroutineScope);
  }
}

void
HostSubroutineOfIndirectLoop::createExecutionPlanExecutionStatements ()
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

  SgVarRefExp
      * colReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::col]);

  SgExpression * initializationExpression = buildAssignOp (colReference,
      buildIntVal (0));

  /*
   * ======================================================
   * The loop stops counting at the number of colours in
   * the plan, minus one
   * ======================================================
   */

  SgVarRefExp * ncolors_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::ncolors,
      subroutineScope);

  SgVarRefExp
      * actualPlanReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan]);

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
      localVariableDeclarations[CUDA::Fortran::VariableNames::blocksPerGrid]);

  SgVarRefExp
      * ncolblkReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::ncolblk]);

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
      localVariableDeclarations[CUDA::Fortran::VariableNames::threadsPerBlock]);

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

  SgVarRefExp
      * sharedMemorySizeReference =
          buildVarRefExp (
              localVariableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]);

  SgVarRefExp * nshared_Reference = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::HostSubroutine::FieldNames::nshared,
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

  SgVarRefExp
      * argsSizesReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]);

  kernelParameters->append_expression (argsSizesReference);

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * OP_DAT_argumentReference = buildVarRefExp (
          localVariableDeclarations[get_OP_DAT_DeviceVariableName (i)]);

      kernelParameters->append_expression (OP_DAT_argumentReference);

      SgVarRefExp * IndirectMap_argumentReference = buildVarRefExp (
          localVariableDeclarations[getLocalToGlobalMappingVariableName (i)]);

      kernelParameters->append_expression (IndirectMap_argumentReference);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * OP_MAP_argumentReference = buildVarRefExp (
          localVariableDeclarations[getGlobalToLocalMappingVariableName (i)]);

      kernelParameters->append_expression (OP_MAP_argumentReference);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == DIRECT)
    {
      SgVarRefExp * OP_DAT_argumentReference = buildVarRefExp (
          localVariableDeclarations[get_OP_DAT_DeviceVariableName (i)]);

      kernelParameters->append_expression (OP_DAT_argumentReference);
    }
  }

  SgVarRefExp
      * pindSizesReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizes]);

  kernelParameters->append_expression (pindSizesReference);

  SgVarRefExp
      * pindOffsReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffs]);

  kernelParameters->append_expression (pindOffsReference);

  SgVarRefExp
      * pblkMapReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMap]);

  kernelParameters->append_expression (pblkMapReference);

  SgVarRefExp
      * poffsetReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::poffset]);

  kernelParameters->append_expression (poffsetReference);

  SgVarRefExp
      * pnelemsReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pnelems]);

  kernelParameters->append_expression (pnelemsReference);

  SgVarRefExp
      * pnthrcolReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcol]);

  kernelParameters->append_expression (pnthrcolReference);

  SgVarRefExp
      * pthrcolReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcol]);

  kernelParameters->append_expression (pthrcolReference);

  SgVarRefExp
      * blockOffsetReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::blockOffset]);

  kernelParameters->append_expression (blockOffsetReference);

  SgExprStatement
      * statement4 =
          buildFunctionCallStmt (
              kernelSubroutine->getSubroutineName () + "<<<"
                  + ROSEHelper::getFirstVariableName (
                      localVariableDeclarations[CUDA::Fortran::VariableNames::blocksPerGrid])
                  + ", "
                  + ROSEHelper::getFirstVariableName (
                      localVariableDeclarations[CUDA::Fortran::VariableNames::threadsPerBlock])
                  + ", "
                  + ROSEHelper::getFirstVariableName (
                      localVariableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize])
                  + ">>>", buildVoidType (), kernelParameters, subroutineScope);

  /*
   * ======================================================
   * Statement to wait for CUDA threads to synchronize
   * ======================================================
   */

  SgStatement * statement5 = buildThreadSynchroniseFunctionCall ();

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
HostSubroutineOfIndirectLoop::initialiseVariablesAndConstants ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageInterface::appendStatement;

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::blockOffset],
      buildIntVal (0), subroutineScope);

  SgExprListExp * initialseConstantsParameters = buildExprListExp ();

  SgExprStatement * initialseConstantsCall = buildFunctionCallStmt (
      initialiseConstantsSubroutine->getSubroutineName (), buildVoidType (),
      initialseConstantsParameters, subroutineScope);

  appendStatement (initialseConstantsCall, subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createPlanCToForttranPointerConversionStatements ()
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

  SgVarRefExp
      * actualPlanReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan]);

  SgVarRefExp
      * indsReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::inds]);

  SgVarRefExp
      * indsNumberReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber]);

  SgVarRefExp
      * ncolblkReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::ncolblk]);

  SgVarRefExp * opSetFormalArgumentReference = buildVarRefExp (
      formalParameterDeclarations[get_OP_SET_FormalParameterName ()]);

  SgVarRefExp
      * pblkMapReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMap]);

  SgVarRefExp
      * pblkMapSizeReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize]);

  SgVarRefExp
      * planRetReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet]);

  SgVarRefExp
      * pmapsReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps]);

  SgVarRefExp
      * pnindirectReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pnindirect]);

  SgVarRefExp
      * pindMapsReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindMaps]);

  SgVarRefExp
      * pindSizesReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizes]);

  SgVarRefExp
      * pindSizesSizeReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize]);

  SgVarRefExp
      * pindOffsReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffs]);

  SgVarRefExp
      * pindOffsSizeReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffsSize]);

  SgVarRefExp
      * pnelemsReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pnelems]);

  SgVarRefExp
      * pnelemsSizeReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pnelemsSize]);

  SgVarRefExp
      * pnthrcolReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcol]);

  SgVarRefExp
      * pnthrcolSizeReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcolSize]);

  SgVarRefExp
      * poffsetReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::poffset]);

  SgVarRefExp
      * poffsetSizeReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::poffsetSize]);

  SgVarRefExp
      * pthrcolReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcol]);

  SgVarRefExp
      * pthrcolSizeReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize]);

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
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber],
              subroutineScope);

  SgExpression
      * argsNumberShapeExpression =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber],
              subroutineScope);

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
      buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::maps,
          subroutineScope)), pmapsReference, argsNumberShapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer call
   * ======================================================
   */

  callStatement = createCallToC_F_POINTER (buildDotExp (actualPlanReference,
      buildOpaqueVarRefExp (
          IndirectLoop::Fortran::HostSubroutine::FieldNames::maps,
          subroutineScope)), pmapsReference, argsNumberShapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * New c_f_pointer calls
   * ======================================================
   */

  unsigned int countIndirectArgs = 1;
  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      SgExpression * indexExpression = buildIntVal (countIndirectArgs++);

      SgExpression * pindMapsArraySubscriptReference = buildPntrArrRefExp (
          pindMapsReference, indexExpression);

      SgExpression * pnindirectArraySubscriptReference = buildPntrArrRefExp (
          pnindirectReference, indexExpression);

      callStatement
          = createCallToC_F_POINTER (
              pindMapsArraySubscriptReference,
              buildVarRefExp (
                  localVariableDeclarations[getLocalToGlobalMappingVariableName (
                      i)]), pnindirectArraySubscriptReference);

      appendStatement (callStatement, subroutineScope);
    }
  }

  /*
   * ======================================================
   * New c_f_pointer calls
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
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

      SgExpression
          * expression1 =
              buildAssignOp (
                  buildVarRefExp (
                      localVariableDeclarations[getGlobalToLocalMappingSizeVariableName (
                          i)]), buildDotExp (opSetFormalArgumentReference,
                      size_Reference));

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
          localVariableDeclarations[getGlobalToLocalMappingVariableName (i)]);

      SgExpression
          * shapeExpression =
              FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                  localVariableDeclarations[getGlobalToLocalMappingVariableName (
                      i)], subroutineScope);

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
      ncolblkReference), ncolblkReference, buildDotExp (
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
      = createCallToC_F_POINTER (
          buildDotExp (actualPlanReference, ind_sizes_Reference),
          pindSizesReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize],
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
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize]));

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  callStatement
      = createCallToC_F_POINTER (
          buildDotExp (actualPlanReference, ind_offs_Reference),
          pindOffsReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffsSize],
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
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize],
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
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::poffsetSize],
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
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pnelemsSize],
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
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcolSize],
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

  callStatement
      = createCallToC_F_POINTER (
          buildDotExp (actualPlanReference, thrcol_Reference),
          pthrcolReference,
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize],
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

  SgVarRefExp * parameter1 = buildVarRefExp (
      formalParameterDeclarations[getUserSubroutineFormalParameterName ()]);

  SgVarRefExp * indexField = buildOpaqueVarRefExp ("index", subroutineScope);

  SgExpression * parameter2 = buildDotExp (buildVarRefExp (
      formalParameterDeclarations[get_OP_SET_FormalParameterName ()]),
      indexField);

  SgVarRefExp
      * parameter3 =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber]);

  SgVarRefExp
      * parameter4 =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::args]);

  SgVarRefExp
      * parameter5 =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::idxs]);

  SgVarRefExp
      * parameter6 =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::maps]);

  SgVarRefExp
      * parameter7 =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::accesses]);

  SgVarRefExp
      * parameter8 =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber]);

  SgVarRefExp
      * parameter9 =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::inds]);

  SgExprListExp * cplan_ActualParameters = buildExprListExp (parameter1,
      parameter2, parameter3, parameter4, parameter5, parameter6, parameter7,
      parameter8, parameter9);

  SgFunctionSymbol * cplanFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("cplan", subroutineScope);

  SgFunctionCallExp * cplanFunctionCall = buildFunctionCallExp (
      cplanFunctionSymbol, cplan_ActualParameters);

  SgVarRefExp
      * cplanFunctionReturnReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet]);

  SgExpression * assignmentExpression = buildAssignOp (
      cplanFunctionReturnReference, cplanFunctionCall);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createDoLoopToCorrectIndexing ()
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
  SgVarRefExp
      * iterationCounterReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::iterationCounter]);

  SgExpression * initializationExpression = buildAssignOp (
      iterationCounterReference, buildIntVal (1));

  /*
   * ======================================================
   * The loop stops counting at the number of OP_DAT argument
   * groups
   * ======================================================
   */
  SgExpression * upperBoundExpression = buildIntVal (
      parallelLoop->getNumberOf_OP_DAT_ArgumentGroups ());

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
  SgVarRefExp
      * opIndirectionArrayReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::idxs]);

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
HostSubroutineOfIndirectLoop::createExecutionPlanStatements ()
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

  int const INDS_UNDEFINED = -2;
  int const NO_INDS = -1;
  int const INDS_BASE_VAL = 0;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opDatFormalArgumentReference = buildVarRefExp (
        formalParameterDeclarations[get_OP_DAT_FormalParameterName (i)]);

    SgExpression * indexField = buildDotExp (opDatFormalArgumentReference,
        buildOpaqueVarRefExp ("index", subroutineScope));

    SgVarRefExp
        * opDatArrayReference =
            buildVarRefExp (
                localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::args]);

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
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opIndexFormalArgumentReference =
        buildVarRefExp (
            formalParameterDeclarations[get_OP_INDIRECTION_FormalParameterName (
                i)]);

    SgVarRefExp
        * opIndirectionArrayReference =
            buildVarRefExp (
                localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::idxs]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opIndirectionArrayReference, indexExpression);

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        opIndexFormalArgumentReference);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);
  }

  createDoLoopToCorrectIndexing ();

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opMapFormalArgumentReference = buildVarRefExp (
        formalParameterDeclarations[get_OP_MAP_FormalParameterName (i)]);

    SgExpression * indexField = buildDotExp (opMapFormalArgumentReference,
        buildOpaqueVarRefExp ("index", subroutineScope));

    SgVarRefExp
        * opMapArrayReference =
            buildVarRefExp (
                localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::maps]);

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
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opAccessFormalArgumentReference = buildVarRefExp (
        formalParameterDeclarations[get_OP_ACCESS_FormalParameterName (i)]);

    SgVarRefExp
        * opAccessArrayReference =
            buildVarRefExp (
                localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::accesses]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opAccessArrayReference, indexExpression);

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        opAccessFormalArgumentReference);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);
  }

  /*
   * ======================================================
   * Set up a mapping between OP_DAT names and indirection
   * values. At the beginning everything is set to
   * INDS_UNDEFINED
   * ======================================================
   */
  std::map <std::string, int> indsValuesPerOPDat;
  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    indsValuesPerOPDat[parallelLoop->get_OP_DAT_VariableName (i)]
        = INDS_UNDEFINED;
  }

  /*
   * ======================================================
   * Must start at the value defined by Mike Giles in his
   * implementation
   * ======================================================
   */
  unsigned int indValuesGenerator = INDS_BASE_VAL;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp
        * indsArrayReference =
            buildVarRefExp (
                localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::inds]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        indsArrayReference, indexExpression);

    SgExpression * rhsExpression;

    if (parallelLoop->get_OP_MAP_Value (i) == DIRECT)
    {
      rhsExpression = buildIntVal (NO_INDS);
      indsValuesPerOPDat[parallelLoop->get_OP_DAT_VariableName (i)] = NO_INDS;
    }
    else
    {
      if (indsValuesPerOPDat[parallelLoop->get_OP_DAT_VariableName (i)]
          == INDS_UNDEFINED)
      {
        rhsExpression = buildIntVal (indValuesGenerator);

        indsValuesPerOPDat[parallelLoop->get_OP_DAT_VariableName (i)]
            = indValuesGenerator;

        indValuesGenerator++;
      }
      else
      {
        rhsExpression = buildIntVal (
            indsValuesPerOPDat[parallelLoop->get_OP_DAT_VariableName (i)]);
      }
    }

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        rhsExpression);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);
  }

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber],
      buildIntVal (parallelLoop->getNumberOf_OP_DAT_ArgumentGroups ()),
      subroutineScope);

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber],
      buildIntVal (parallelLoop->getNumberOfDistinctIndirect_OP_DAT_Arguments ()),
      subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createExecutionPlanLocalVariables ()
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

  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::VariableNames::argsSizes,
          dataSizesDeclarationOfIndirectLoop->getType (), subroutineScope, 1,
          DEVICE);

  /*
   * ======================================================
   * Create pointers to the execution plan on the C and
   * Fortran side
   * ======================================================
   */

  SgType * c_ptrType = FortranTypesBuilder::buildNewTypeDeclaration ("c_ptr",
      subroutineScope)->get_type ();

  localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet,
          c_ptrType, subroutineScope);

  SgType * op_planType = FortranTypesBuilder::buildNewTypeDeclaration (
      "op_plan", subroutineScope)->get_type ();

  localVariableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan,
          buildPointerType (op_planType), subroutineScope);

  /*
   * ======================================================
   * Create pointer to the
   * ======================================================
   */

  SgType * c_devptrType = FortranTypesBuilder::buildNewTypeDeclaration (
      "c_devptr", subroutineScope)->get_type ();

  localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindMaps]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindMaps,
          buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope);

  localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pmaps,
          buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope);

  localVariableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindMapsSize]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::PlanFunction::VariableNames::pindMapsSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = getLocalToGlobalMappingVariableName (i);

      localVariableDeclarations[variableName]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getArray_RankOne (
                  FortranTypesBuilder::getFourByteInteger ()), subroutineScope,
              2, DEVICE, ALLOCATABLE);
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
    localVariableDeclarations[*it]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            *it, FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger (), 1,
                parallelLoop->getNumberOf_OP_DAT_ArgumentGroups ()),
            subroutineScope);
  }

  /*
   * ======================================================
   * Create variables modelling the indirect mappings to
   * local indices in shared memory
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = getGlobalToLocalMappingVariableName (i);

      localVariableDeclarations[variableName]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getArray_RankOne (
                  FortranTypesBuilder::getTwoByteInteger ()), subroutineScope,
              2, DEVICE, ALLOCATABLE);
    }
  }

  /*
   * ======================================================
   * Create variables modelling the number of elements
   * in each block of an indirect mapping
   * ======================================================
   */
  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = getGlobalToLocalMappingSizeVariableName (i);

      localVariableDeclarations[variableName]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);
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

  /*
   * ======================================================
   * threadSynchRet is used by both direct and indirect
   * loops, then it is now moved to the
   * createCUDAKernelVariables routine
   * ======================================================
   */

  for (vector <string>::iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    localVariableDeclarations[*it]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            *it, FortranTypesBuilder::getFourByteInteger (), subroutineScope);
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
    localVariableDeclarations[*it]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            *it, buildPointerType (FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ())), subroutineScope);
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
    localVariableDeclarations[*it]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            *it, FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ()), subroutineScope,
            2, DEVICE, ALLOCATABLE);
  }
}

void
HostSubroutineOfIndirectLoop::createStatements ()
{

}

void
HostSubroutineOfIndirectLoop::createLocalVariableDeclarations ()
{

}

void
HostSubroutineOfIndirectLoop::createFormalParameterDeclarations ()
{

}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

HostSubroutineOfIndirectLoop::HostSubroutineOfIndirectLoop (
    std::string const & subroutineName,
    UserDeviceSubroutine * userDeviceSubroutine,
    KernelSubroutine * kernelSubroutine,
    InitialiseConstantsSubroutine * initialiseConstantsSubroutine,
    DataSizesDeclarationOfIndirectLoop * dataSizesDeclarationOfIndirectLoop,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  HostSubroutine (subroutineName, userDeviceSubroutine, kernelSubroutine,
      parallelLoop, moduleScope)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", 2);

  this->initialiseConstantsSubroutine = initialiseConstantsSubroutine;

  this->dataSizesDeclarationOfIndirectLoop = dataSizesDeclarationOfIndirectLoop;

  createFormalParameters ();

  createDataMarshallingLocalVariables ();

  createExecutionPlanLocalVariables ();

  createCUDAKernelVariables ();

  initialiseDataMarshallingLocalVariables ();

  createExecutionPlanStatements ();

  createPlanFunctionCallStatement ();

  createPlanCToForttranPointerConversionStatements ();

  initialiseDeviceVariablesSizesVariable ();

  initialiseVariablesAndConstants ();

  createExecutionPlanExecutionStatements ();

  copyDataBackFromDeviceAndDeallocate ();
}
