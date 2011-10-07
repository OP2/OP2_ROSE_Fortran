#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranParallelLoop.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranCUDAModuleDeclarations.h>
#include <FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <CUDA.h>
#include <Debug.h>

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

  Debug::getInstance ()->debugMessage (
      "Creating CUDA kernel function call statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (buildVarRefExp (
      moduleDeclarations->getDimensionsVariableDeclaration ()));

  actualParameters->append_expression (buildVarRefExp (
      moduleDeclarations->getDataSizesVariableDeclaration ()));

  Debug::getInstance ()->debugMessage (
      "Adding OP_DAT parameters with indirect access", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isIndirect (i))
    {
      actualParameters->append_expression (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatDeviceName (i))));

      actualParameters->append_expression (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getLocalToGlobalMappingName (i))));
    }
  }

  Debug::getInstance ()->debugMessage (
      "Adding global to local memory remapping parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      actualParameters->append_expression (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getGlobalToLocalMappingName (i))));
    }
  }

  Debug::getInstance ()->debugMessage (
      "Adding direct and non-scalar global parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i) || parallelLoop->isGlobalArray (i))
      {
        actualParameters->append_expression (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::getOpDatDeviceName (
                i))));
      }
    }
  }

  Debug::getInstance ()->debugMessage ("Adding plan function parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pindSizes)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pindOffs)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pblkMap)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::poffset)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pnelems)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pnthrcol)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pthrcol)));

  actualParameters->append_expression (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::blockOffset)));

  SgExprStatement * callStatement = buildFunctionCallStmt (kernelSubroutineName
      + "<<<" + RoseHelper::getFirstVariableName (variableDeclarations->get (
      CUDA::blocksPerGrid)) + ", " + RoseHelper::getFirstVariableName (
      variableDeclarations->get (CUDA::threadsPerBlock)) + ", "
      + RoseHelper::getFirstVariableName (variableDeclarations->get (
          CUDA::sharedMemorySize)) + ">>>", buildVoidType (), actualParameters,
      subroutineScope);

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
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;
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
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::blockOffset)), buildIntVal (0));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Statement to initialise the grid dimension of the
   * CUDA kernel
   * ======================================================
   */

  SgExpression * arrayIndexExpression1 =
      buildAddOp (buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::colour1)), buildIntVal (1));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::ncolblk)),
      arrayIndexExpression1);

  SgExprStatement * statement1 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (CUDA::blocksPerGrid)), arrayExpression1);

  appendStatement (statement1, loopBody);

  /*
   * ======================================================
   * Statement to initialise the number of threads per block
   * in the CUDA kernel
   * ======================================================
   */

  SgExprStatement * statement2 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (CUDA::threadsPerBlock)), buildOpaqueVarRefExp (
      OP2::VariableNames::threadBlockSizeMacro, subroutineScope));

  appendStatement (statement2, loopBody);

  /*
   * ======================================================
   * Statement to initialise the shared memory size in the
   * CUDA kernel
   * ======================================================
   */

  SgDotExp * dotExpression3 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nshared, subroutineScope));

  SgExprStatement * statement3 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (CUDA::sharedMemorySize)), dotExpression3);

  appendStatement (statement3, loopBody);

  appendStatement (createKernelFunctionCallStatement (), loopBody);

  /*
   * ======================================================
   * Statement to synchronise the CUDA threads
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (CUDA::threadSynchRet)),
      CUDA::createHostThreadSynchronisationCallStatement (subroutineScope));

  appendStatement (assignmentStatement2, loopBody);

  /*
   * ======================================================
   * Statement to increment the block offset
   * ======================================================
   */

  SgAddOp * addExpression4 = buildAddOp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::blockOffset)), buildVarRefExp (
          variableDeclarations->get (CUDA::blocksPerGrid)));

  SgStatement * statement4 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::blockOffset)), addExpression4);

  appendStatement (statement4, loopBody);

  /*
   * ======================================================
   * The loop starts counting from 0
   * ======================================================
   */

  SgAssignOp * initializationExpression =
      buildAssignOp (buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::colour1)), buildIntVal (0));

  /*
   * ======================================================
   * The loop stops counting at the number of colours in
   * the plan, minus one
   * ======================================================
   */

  SgDotExp * dotExpression = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::ncolors, subroutineScope));

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
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildBoolValExp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise variable sizes",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * ifBody = buildBasicBlock ();

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (
          moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (false));

  appendStatement (assignmentStatement1, ifBody);

  unsigned int countIndirectArgs = 1;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
            moduleDeclarations->getDataSizesVariableDeclaration ()),
            buildVarRefExp (dataSizesDeclaration->getFieldDeclarations ()->get (
                OP2::VariableNames::getLocalToGlobalMappingSizeName (i))));

        SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::PlanFunction::pnindirect)), buildIntVal (
                countIndirectArgs));

        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression, arrayIndexExpression);

        appendStatement (assignmentStatement, ifBody);

        countIndirectArgs++;
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      SgDotExp * fieldSelectionExpression = buildDotExp (buildVarRefExp (
          moduleDeclarations->getDataSizesVariableDeclaration ()),
          buildVarRefExp (dataSizesDeclaration->getFieldDeclarations ()->get (
              OP2::VariableNames::getGlobalToLocalMappingSizeName (i))));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          fieldSelectionExpression, buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getGlobalToLocalMappingSizeName (i))));

      appendStatement (assignmentStatement, ifBody);
    }
  }

  vector <string> planFunctionSizeVariables;

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pblkMapSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pindOffsSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pindSizesSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pnelemsSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pnthrcolSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::poffsetSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pthrcolSize);

  for (vector <string>::iterator it = planFunctionSizeVariables.begin (); it
      != planFunctionSizeVariables.end (); ++it)
  {
    SgDotExp * fieldSelectionExpression = buildDotExp (buildVarRefExp (
        moduleDeclarations->getDataSizesVariableDeclaration ()),
        buildVarRefExp (
            dataSizesDeclaration->getFieldDeclarations ()->get (*it)));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        fieldSelectionExpression, buildVarRefExp (variableDeclarations->get (
            *it)));

    appendStatement (assignmentStatement, ifBody);
  }

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (true));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, subroutineScope);
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

  Debug::getInstance ()->debugMessage ("Creating execution plan variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Create pointer to the execution plan
   * ======================================================
   */

  SgType * op_planType = FortranTypesBuilder::buildClassDeclaration ("op_plan",
      subroutineScope)->get_type ();

  variableDeclarations->add (OP2::VariableNames::PlanFunction::actualPlan,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::actualPlan, buildPointerType (
              op_planType), subroutineScope));

  /*
   * ======================================================
   * Create pointer to the
   * ======================================================
   */

  SgType * c_devptrType = FortranTypesBuilder::buildClassDeclaration (
      "c_devptr", subroutineScope)->get_type ();

  variableDeclarations->add (OP2::VariableNames::PlanFunction::pindMaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::pindMaps, buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope));

  variableDeclarations->add (OP2::VariableNames::PlanFunction::pmaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::pmaps, buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope));

  variableDeclarations->add (OP2::VariableNames::PlanFunction::pindMapsSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::pindMapsSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const variableName =
            OP2::VariableNames::getLocalToGlobalMappingName (i);

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

  fourByteIntegerArrays.push_back (OP2::VariableNames::PlanFunction::args);

  fourByteIntegerArrays.push_back (OP2::VariableNames::PlanFunction::idxs);

  fourByteIntegerArrays.push_back (OP2::VariableNames::PlanFunction::maps);

  fourByteIntegerArrays.push_back (OP2::VariableNames::PlanFunction::accesses);

  fourByteIntegerArrays.push_back (OP2::VariableNames::PlanFunction::inds);

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
      string const variableName =
          OP2::VariableNames::getGlobalToLocalMappingName (i);

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
      string const variableName =
          OP2::VariableNames::getGlobalToLocalMappingSizeName (i);

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

  fourByteIntegerVariables.push_back (OP2::VariableNames::colour1);

  fourByteIntegerVariables.push_back (CommonVariableNames::iterationCounter1);

  fourByteIntegerVariables.push_back (
      OP2::VariableNames::PlanFunction::argsNumber);

  fourByteIntegerVariables.push_back (
      OP2::VariableNames::PlanFunction::indsNumber);

  fourByteIntegerVariables.push_back (
      OP2::VariableNames::PlanFunction::blockOffset);

  fourByteIntegerVariables.push_back (
      OP2::VariableNames::PlanFunction::pindSizesSize);

  fourByteIntegerVariables.push_back (
      OP2::VariableNames::PlanFunction::pindOffsSize);

  fourByteIntegerVariables.push_back (
      OP2::VariableNames::PlanFunction::pblkMapSize);

  fourByteIntegerVariables.push_back (
      OP2::VariableNames::PlanFunction::poffsetSize);

  fourByteIntegerVariables.push_back (
      OP2::VariableNames::PlanFunction::pnelemsSize);

  fourByteIntegerVariables.push_back (
      OP2::VariableNames::PlanFunction::pnthrcolSize);

  fourByteIntegerVariables.push_back (
      OP2::VariableNames::PlanFunction::pthrcolSize);

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

  integerPointerVariables.push_back (OP2::VariableNames::PlanFunction::ncolblk);

  integerPointerVariables.push_back (
      OP2::VariableNames::PlanFunction::pnindirect);

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
      OP2::VariableNames::PlanFunction::pindSizes);

  deviceIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pindOffs);

  deviceIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pblkMap);

  deviceIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::poffset);

  deviceIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pnelems);

  deviceIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pnthrcol);

  deviceIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pthrcol);

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
FortranCUDAHostSubroutineIndirectLoop::createStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildBoolValExp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (true));

  SgBasicBlock * ifBody = createFirstTimeExecutionStatements ();

  appendStatement (createPlanFunctionParametersPreparationStatements (
      (FortranParallelLoop *) parallelLoop, variableDeclarations), ifBody);

  SgFunctionCallExp * planFunctionCallExpression =
      createPlanFunctionCallExpression (subroutineScope, variableDeclarations);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (moduleDeclarations->getCPlanReturnDeclaration ()),
      planFunctionCallExpression);

  appendStatement (assignmentStatement1, ifBody);

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, subroutineScope);

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionPrologueStatements ();
  }

  SgStatement * callStatement1 =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, buildVarRefExp (
              moduleDeclarations->getCPlanReturnDeclaration ()),
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::PlanFunction::actualPlan)));

  appendStatement (callStatement1, subroutineScope);

  appendStatement (createConvertPositionInPMapsStatements (
      (FortranParallelLoop *) parallelLoop, subroutineScope,
      variableDeclarations), subroutineScope);

  appendStatement (createConvertPlanFunctionParametersStatements (
      (FortranParallelLoop *) parallelLoop, subroutineScope,
      variableDeclarations), subroutineScope);

  createVariablesSizesInitialisationStatements ();

  createPlanFunctionExecutionStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionEpilogueStatements ();
  }
}

void
FortranCUDAHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
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
    FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranCUDAHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope, dataSizesDeclaration,
      opDatDimensionsDeclaration, moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
