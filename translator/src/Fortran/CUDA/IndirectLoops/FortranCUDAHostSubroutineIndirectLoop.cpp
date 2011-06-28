#include <Debug.h>
#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <ROSEHelper.h>

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
      variableDeclarations->get (CommonVariableNames::opDatDimensions)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::argsSizes)));

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
      variableDeclarations->get (PlanFunction::Fortran::pindSizes)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pindOffs)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pblkMap)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::poffset)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pnelems)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pnthrcol)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pthrcol)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::blockOffset)));

  SgExprStatement * callStatement = buildFunctionCallStmt (kernelSubroutineName
      + "<<<" + ROSEHelper::getFirstVariableName (variableDeclarations->get (
      CUDA::Fortran::blocksPerGrid)) + ", " + ROSEHelper::getFirstVariableName (
      variableDeclarations->get (CUDA::Fortran::threadsPerBlock)) + ", "
      + ROSEHelper::getFirstVariableName (variableDeclarations->get (
          CUDA::Fortran::sharedMemorySize)) + ">>>", buildVoidType (),
      actualParameters, subroutineScope);

  return callStatement;
}

void
FortranCUDAHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
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
   * Statement to initialise the block offset
   * ======================================================
   */

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::blockOffset)),
      buildIntVal (0));

  appendStatement (assignmentStatement, subroutineScope);

  /*
   * ======================================================
   * Statement to initialise the grid dimension of the
   * CUDA kernel
   * ======================================================
   */

  SgExpression * arrayIndexExpression1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::col)), buildIntVal (1));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::ncolblk)),
      arrayIndexExpression1);

  SgExprStatement * statement1 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::blocksPerGrid)),
      arrayExpression1);

  appendStatement (statement1, loopBody);

  /*
   * ======================================================
   * Statement to initialise the number of threads per block
   * in the CUDA kernel
   * ======================================================
   */

  SgExprStatement * statement2 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::threadsPerBlock)),
      buildOpaqueVarRefExp ("FOP_BLOCK_SIZE", subroutineScope));

  appendStatement (statement2, loopBody);

  /*
   * ======================================================
   * Statement to initialise the shared memory size in the
   * CUDA kernel
   * ======================================================
   */

  SgDotExp * dotExpression3 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::nshared, subroutineScope));

  SgExprStatement * statement3 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::sharedMemorySize)),
      dotExpression3);

  appendStatement (statement3, loopBody);

  appendStatement (createKernelFunctionCallStatement (), loopBody);

  appendStatement (createThreadSynchroniseCallStatement (), loopBody);

  /*
   * ======================================================
   * Statement to increment the block offset
   * ======================================================
   */

  SgAddOp * addExpression4 =
      buildAddOp (buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::blockOffset)), buildVarRefExp (
          variableDeclarations->get (CUDA::Fortran::blocksPerGrid)));

  SgStatement * statement4 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::blockOffset)),
      addExpression4);

  appendStatement (statement4, loopBody);

  /*
   * ======================================================
   * The loop starts counting from 0
   * ======================================================
   */

  SgAssignOp * initializationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::col)), buildIntVal (0));

  /*
   * ======================================================
   * The loop stops counting at the number of colours in
   * the plan, minus one
   * ======================================================
   */

  SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::ncolors, subroutineScope));

  SgExpression * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  appendStatement (loopStatement, subroutineScope);
}

void
FortranCUDAHostSubroutineIndirectLoop::createVariablesSizesInitialisationStatements ()
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
          variableDeclarations->get (CommonVariableNames::argsSizes));

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
          variableDeclarations->get (CommonVariableNames::argsSizes));

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclarationOfIndirectLoop->getFieldDeclarations ()->get (
              VariableNames::getLocalToGlobalMappingSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgVarRefExp * pnindirect_Reference = buildVarRefExp (
          variableDeclarations->get (PlanFunction::Fortran::pnindirect));

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
          variableDeclarations->get (CommonVariableNames::argsSizes));

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
          variableDeclarations->get (CommonVariableNames::argsSizes));

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

  planFunctionSizeVariables.push_back (PlanFunction::Fortran::pblkMapSize);

  planFunctionSizeVariables.push_back (PlanFunction::Fortran::pindOffsSize);

  planFunctionSizeVariables.push_back (PlanFunction::Fortran::pindSizesSize);

  planFunctionSizeVariables.push_back (PlanFunction::Fortran::pnelemsSize);

  planFunctionSizeVariables.push_back (PlanFunction::Fortran::pnthrcolSize);

  planFunctionSizeVariables.push_back (PlanFunction::Fortran::poffsetSize);

  planFunctionSizeVariables.push_back (PlanFunction::Fortran::pthrcolSize);

  for (vector <string>::iterator it = planFunctionSizeVariables.begin (); it
      != planFunctionSizeVariables.end (); ++it)
  {
    SgVarRefExp * dataSizesReferences = buildVarRefExp (
        variableDeclarations->get (CommonVariableNames::argsSizes));

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

  variableDeclarations->add (PlanFunction::Fortran::planRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::Fortran::planRet, c_ptrType, subroutineScope));

  SgType * op_planType = FortranTypesBuilder::buildNewTypeDeclaration (
      "op_plan", subroutineScope)->get_type ();

  variableDeclarations->add (PlanFunction::Fortran::actualPlan,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::Fortran::actualPlan, buildPointerType (op_planType),
          subroutineScope));

  /*
   * ======================================================
   * Create pointer to the
   * ======================================================
   */

  SgType * c_devptrType = FortranTypesBuilder::buildNewTypeDeclaration (
      "c_devptr", subroutineScope)->get_type ();

  variableDeclarations->add (PlanFunction::Fortran::pindMaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::Fortran::pindMaps, buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope));

  variableDeclarations->add (PlanFunction::Fortran::pmaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::Fortran::pmaps, buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope));

  variableDeclarations->add (PlanFunction::Fortran::pindMapsSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::Fortran::pindMapsSize,
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

  fourByteIntegerArrays.push_back (PlanFunction::Fortran::args);

  fourByteIntegerArrays.push_back (PlanFunction::Fortran::idxs);

  fourByteIntegerArrays.push_back (PlanFunction::Fortran::maps);

  fourByteIntegerArrays.push_back (PlanFunction::Fortran::accesses);

  fourByteIntegerArrays.push_back (PlanFunction::Fortran::inds);

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

  fourByteIntegerVariables.push_back (CommonVariableNames::col);

  fourByteIntegerVariables.push_back (CommonVariableNames::iterationCounter1);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::argsNumber);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::indsNumber);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::blockOffset);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::pindSizesSize);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::pindOffsSize);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::pblkMapSize);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::poffsetSize);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::pnelemsSize);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::pnthrcolSize);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::pthrcolSize);

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

  integerPointerVariables.push_back (PlanFunction::Fortran::ncolblk);

  integerPointerVariables.push_back (PlanFunction::Fortran::pnindirect);

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

  deviceIntegerArrayVariables.push_back (PlanFunction::Fortran::pindSizes);

  deviceIntegerArrayVariables.push_back (PlanFunction::Fortran::pindOffs);

  deviceIntegerArrayVariables.push_back (PlanFunction::Fortran::pblkMap);

  deviceIntegerArrayVariables.push_back (PlanFunction::Fortran::poffset);

  deviceIntegerArrayVariables.push_back (PlanFunction::Fortran::pnelems);

  deviceIntegerArrayVariables.push_back (PlanFunction::Fortran::pnthrcol);

  deviceIntegerArrayVariables.push_back (PlanFunction::Fortran::pthrcol);

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
  using SageInterface::appendStatement;

  createOpDatDimensionInitialisationStatements ();

  createTransferOpDatStatements (subroutineScope);

  createCUDAKernelPrologueStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionPrologueStatements ();
  }

  createPlanFunctionParametersPreparationStatements (variableDeclarations,
      parallelLoop, subroutineScope);

  createPlanFunctionCallStatement (variableDeclarations, subroutineScope,
      subroutineScope);

  createConvertPositionInPMapsStatements (variableDeclarations, parallelLoop,
      subroutineScope, subroutineScope);

  createConvertPlanFunctionParametersStatements (variableDeclarations,
      parallelLoop, subroutineScope, subroutineScope);

  createVariablesSizesInitialisationStatements ();

  appendStatement (createInitialiseConstantsCallStatement (), subroutineScope);

  createPlanFunctionExecutionStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionEpilogueStatements ();
  }

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

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionLocalVariableDeclarations ();
  }
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
      initialiseConstantsSubroutine, opDatDimensionsDeclaration),
      dataSizesDeclarationOfIndirectLoop (dataSizesDeclarationOfIndirectLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", 2);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
