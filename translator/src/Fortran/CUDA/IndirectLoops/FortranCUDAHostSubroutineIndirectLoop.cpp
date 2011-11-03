#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranParallelLoop.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranCUDAModuleDeclarations.h>
#include <FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop.h>
#include <FortranPlan.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <CUDA.h>
#include <Debug.h>

SgStatement *
FortranCUDAHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA kernel function call statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (variableDeclarations->getReference (
      OP2::VariableNames::opDatDimensions));

  actualParameters->append_expression (variableDeclarations->getReference (
      OP2::VariableNames::opDatCardinalities));

  Debug::getInstance ()->debugMessage ("Adding OP_DAT parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i))
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (
                OP2::VariableNames::getReductionArrayDeviceName (i)));
      }
      else if (parallelLoop->isIndirect (i) || parallelLoop->isDirect (i))
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (
                OP2::VariableNames::getOpDatDeviceName (i)));
      }
      else if (parallelLoop->isRead (i))
      {
        if (parallelLoop->isArray (i))
        {
          actualParameters->append_expression (
              variableDeclarations->getReference (
                  OP2::VariableNames::getOpDatDeviceName (i)));
        }
        else
        {
          actualParameters->append_expression (
              variableDeclarations->getReference (
                  OP2::VariableNames::getOpDatHostName (i)));
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
            variableDeclarations->getReference (
                OP2::VariableNames::getLocalToGlobalMappingName (i)));
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
          OP2::VariableNames::getGlobalToLocalMappingName (i)));
    }
  }

  Debug::getInstance ()->debugMessage ("Adding plan function parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  actualParameters->append_expression (variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::pindSizes));

  actualParameters->append_expression (variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::pindOffs));

  actualParameters->append_expression (variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::pblkMap));

  actualParameters->append_expression (variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::poffset));

  actualParameters->append_expression (variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::pnelems));

  actualParameters->append_expression (variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::pnthrcol));

  actualParameters->append_expression (variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::pthrcol));

  actualParameters->append_expression (variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::blockOffset));

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
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::blockOffset), buildIntVal (0));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Statement to initialise the grid dimension of the
   * CUDA kernel
   * ======================================================
   */

  SgExpression * arrayIndexExpression1 = buildAddOp (
      variableDeclarations->getReference (OP2::VariableNames::colour1),
      buildIntVal (1));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::ncolblk), arrayIndexExpression1);

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
      buildOpaqueVarRefExp (OP2::VariableNames::threadBlockSizeMacro,
          subroutineScope));

  appendStatement (statement2, loopBody);

  /*
   * ======================================================
   * Statement to initialise the shared memory size in the
   * CUDA kernel
   * ======================================================
   */

  SgDotExp * dotExpression3 = buildDotExp (variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
      OP2::VariableNames::PlanFunction::nshared, subroutineScope));

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
      OP2::VariableNames::PlanFunction::blockOffset),
      variableDeclarations->getReference (CUDA::blocksPerGrid));

  SgStatement * statement4 = buildAssignStatement (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::blockOffset), addExpression4);

  appendStatement (statement4, loopBody);

  /*
   * ======================================================
   * The loop starts counting from 0
   * ======================================================
   */

  SgAssignOp * initializationExpression = buildAssignOp (
      variableDeclarations->getReference (OP2::VariableNames::colour1),
      buildIntVal (0));

  /*
   * ======================================================
   * The loop stops counting at the number of colours in
   * the plan, minus one
   * ======================================================
   */

  SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
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
FortranCUDAHostSubroutineIndirectLoop::createCardinalitiesInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
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
            variableDeclarations->getReference (
                OP2::VariableNames::opDatCardinalities),
            cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
                OP2::VariableNames::getLocalToGlobalMappingSizeName (i)));

        SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (
                OP2::VariableNames::PlanFunction::pnindirect), buildIntVal (
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
          variableDeclarations->getReference (
              OP2::VariableNames::opDatCardinalities),
          cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
              OP2::VariableNames::getGlobalToLocalMappingSizeName (i)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          fieldSelectionExpression, variableDeclarations->getReference (
              OP2::VariableNames::getGlobalToLocalMappingSizeName (i)));

      appendStatement (assignmentStatement, subroutineScope);
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
    SgDotExp * fieldSelectionExpression = buildDotExp (
        variableDeclarations->getReference (
            OP2::VariableNames::opDatCardinalities),
        cardinalitiesDeclaration->getFieldDeclarations ()->getReference (*it));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        fieldSelectionExpression, variableDeclarations->getReference (*it));

    appendStatement (assignmentStatement, subroutineScope);
  }
}

void
FortranCUDAHostSubroutineIndirectLoop::createExecutionPlanDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
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

  fourByteIntegerVariables.push_back (
      CommonVariableNames::getIterationCounterVariableName (1));

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
  using namespace SageBuilder;
  using namespace SageInterface;

  appendStatement (
      fortranPlan->createPlanFunctionParametersPreparationStatements (),
      subroutineScope);

  SgFunctionCallExp * planFunctionCallExpression =
      fortranPlan-> createPlanFunctionCallExpression ();

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      moduleDeclarations->getCPlanReturnDeclaration (),
      planFunctionCallExpression);

  appendStatement (assignmentStatement1, subroutineScope);

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionPrologueStatements ();
  }

  SgStatement
      * callStatement1 =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope,
              moduleDeclarations->getCPlanReturnDeclaration (),
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::actualPlan));

  appendStatement (callStatement1, subroutineScope);

  appendStatement (fortranPlan->createConvertPositionInPMapsStatements (),
      subroutineScope);

  appendStatement (
      fortranPlan->createConvertPlanFunctionParametersStatements (),
      subroutineScope);

  createCardinalitiesInitialisationStatements ();

  createPlanFunctionExecutionStatements ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }

  createDeallocateStatements ();
}

void
FortranCUDAHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  createOpDatDimensionsDeclaration ();

  createOpDatCardinalitiesDeclaration ();

  createDataMarshallingDeclarations ();

  createCUDAConfigurationLaunchDeclarations ();

  createExecutionPlanDeclarations ();

  if (parallelLoop->isReductionRequired () == true)
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

  fortranPlan = new FortranPlan (subroutineScope, parallelLoop,
      variableDeclarations);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
