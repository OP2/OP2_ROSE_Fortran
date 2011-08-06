#include <Debug.h>
//#include <CPPOpenCLHostSubroutineIndirectLoop.h>
//#include <CPPTypesBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <CPPPlan.h>

using namespace SageBuilder; //TODO: remove
/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
CPPOpenCLHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildVarRefExp;

  SgExprListExp * actualParameters = buildExprListExp ();
  
  std::vector<std::pair<SgExpression *, SgExpression *> > kernelArguments;

  actualParameters->append_expression (buildVarRefExp (
      moduleDeclarations->getDimensionsVariableDeclaration ()));

  actualParameters->append_expression (buildVarRefExp (
      moduleDeclarations->getDataSizesVariableDeclaration ()));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      actualParameters->append_expression (
          buildVarRefExp (variableDeclarations->get (VariableNames::getOpDatDeviceName (i))));

      actualParameters->append_expression (
          buildVarRefExp ( variableDeclarations->get ( VariableNames::getLocalToGlobalMappingName (i))));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      actualParameters->append_expression (buildVarRefExp (
          variableDeclarations->get ( VariableNames::getGlobalToLocalMappingName (i))));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && (parallelLoop->getOpMapValue (i) == DIRECT
            || parallelLoop->getOpMapValue (i) == GLOBAL))
    {
      actualParameters->append_expression (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatDeviceName (i))));
    }
  }

  actualParameters->append_expression (
      buildDotExp(
          buildVarRefExp( variableDeclarations->get( PlanFunction::CPP::plan ) ),
          buildVarRefExp ( variableDeclarations->get (PlanFunction::CPP::pindSizes ) ) ) );
          
  /*
      buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::pindSizes)));
      */

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::pindOffs)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::pblkMap)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::poffset)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::pnelems)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::pnthrcol)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::pthrcol)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::blockOffset)));

  SgExprStatement * callStatement = buildFunctionCallStmt (kernelSubroutineName
      + "<<<" + RoseHelper::getFirstVariableName (variableDeclarations->get (
      OpenCL::CPP::blocksPerGrid)) + ", " + RoseHelper::getFirstVariableName (
      variableDeclarations->get (OpenCL::CPP::threadsPerBlock)) + ", "
      + RoseHelper::getFirstVariableName (variableDeclarations->get (
          OpenCL::CPP::sharedMemorySize)) + ">>>", buildVoidType (),
      actualParameters, subroutineScope);

  return callStatement;
}

void
CPPOpenCLHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
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

  Debug::getInstance ()->debugMessage (
      "Creating plan function execution statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  /*
   * ======================================================
   * Statement to initialise the block offset
   * ======================================================
   */

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::blockOffset)),
      buildIntVal (0));

  appendStatement (assignmentStatement, subroutineScope);

  /*
   * ======================================================
   * Statement to initialise the grid dimension of the
   * OpenCL kernel
   * ======================================================
   */

  SgExpression * arrayIndexExpression1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::col)), buildIntVal (1));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::ncolblk)),
      arrayIndexExpression1);

  SgExprStatement * statement1 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::blocksPerGrid)),
      arrayExpression1);

  appendStatement (statement1, loopBody);

  /*
   * ======================================================
   * Statement to initialise the number of threads per block
   * in the OpenCL kernel
   * ======================================================
   */

  SgExprStatement * statement2 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::threadsPerBlock)),
      buildOpaqueVarRefExp ("FOP_BLOCK_SIZE", subroutineScope));

  appendStatement (statement2, loopBody);

  /*
   * ======================================================
   * Statement to initialise the shared memory size in the
   * OpenCL kernel
   * ======================================================
   */

  SgDotExp * dotExpression3 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::CPP::nshared, subroutineScope));

  SgExprStatement * statement3 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::sharedMemorySize)),
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
          PlanFunction::CPP::blockOffset)), buildVarRefExp (
          variableDeclarations->get (OpenCL::CPP::blocksPerGrid)));

  SgStatement * statement4 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (PlanFunction::CPP::blockOffset)),
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
      variableDeclarations->get (PlanFunction::CPP::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::CPP::ncolors, subroutineScope));

  SgExpression * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgForStatement * loopStatement = new SgForStatement (
      upperBoundExpression,
      incrementExpression,
      loopBody);
  /*
      CPPStatementsAndExpressionsBuilder::buildCPPDoStatement (
          initializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);
          */

  appendStatement (loopStatement, subroutineScope);
}

void
CPPOpenCLHostSubroutineIndirectLoop::createVariablesSizesInitialisationStatements ()
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
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgVarRefExp * dataSizesReferences = buildVarRefExp (
          moduleDeclarations->getDataSizesVariableDeclaration ());

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclaration->getFieldDeclarations ()->get (
              VariableNames::getLocalToGlobalMappingSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgVarRefExp * pnindirect_Reference = buildVarRefExp (
          variableDeclarations->get (PlanFunction::CPP::pnindirect));

      SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
          pnindirect_Reference, buildIntVal (countIndirectArgs));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          fieldSelectionExpression, arrayIndexExpression);

      appendStatement (assignmentStatement, ifBody);

      countIndirectArgs++;
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgVarRefExp * dataSizesReferences = buildVarRefExp (
          moduleDeclarations->getDataSizesVariableDeclaration ());

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclaration->getFieldDeclarations ()->get (
              VariableNames::getGlobalToLocalMappingSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
          fieldReference);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          fieldSelectionExpression, buildVarRefExp (variableDeclarations->get (
              VariableNames::getGlobalToLocalMappingSizeName (i))));

      appendStatement (assignmentStatement, ifBody);
    }
  }

  vector <string> planFunctionSizeVariables;

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pblkMapSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pindOffsSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pindSizesSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pnelemsSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pnthrcolSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::poffsetSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pthrcolSize);

  for (vector <string>::iterator it = planFunctionSizeVariables.begin (); it
      != planFunctionSizeVariables.end (); ++it)
  {
    SgVarRefExp * dataSizesReferences = buildVarRefExp (
        moduleDeclarations->getDataSizesVariableDeclaration ());

    SgVarRefExp * fieldReference = buildVarRefExp (
        dataSizesDeclaration->getFieldDeclarations ()->get (*it));

    SgDotExp * fieldSelectionExpression = buildDotExp (dataSizesReferences,
        fieldReference);

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
CPPOpenCLHostSubroutineIndirectLoop::createExecutionPlanDeclarations ()
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

  SgType * op_planType = CPPTypesBuilder::buildClassDeclaration ("op_plan",
      subroutineScope)->get_type ();

  variableDeclarations->add (PlanFunction::CPP::actualPlan,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::CPP::actualPlan, buildPointerType (op_planType),
          subroutineScope));

  /*
   * ======================================================
   * Create pointer to the
   * ======================================================
   */

  SgType * c_devptrType = CPPTypesBuilder::buildClassDeclaration (
      "c_devptr", subroutineScope)->get_type ();

  variableDeclarations->add (PlanFunction::CPP::pindMaps,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::CPP::pindMaps, buildPointerType (
              CPPTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope));

  variableDeclarations->add (PlanFunction::CPP::pmaps,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::CPP::pmaps, buildPointerType (
              CPPTypesBuilder::getArray_RankOne (c_devptrType)),
          subroutineScope));

  variableDeclarations->add (PlanFunction::CPP::pindMapsSize,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::CPP::pindMapsSize,
          CPPTypesBuilder::getFourByteInteger (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          VariableNames::getLocalToGlobalMappingName (i);

      variableDeclarations->add (variableName,
          CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, CPPTypesBuilder::getArray_RankOne (
                  CPPTypesBuilder::getFourByteInteger ()), subroutineScope,
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

  fourByteIntegerArrays.push_back (PlanFunction::CPP::args);

  fourByteIntegerArrays.push_back (PlanFunction::CPP::idxs);

  fourByteIntegerArrays.push_back (PlanFunction::CPP::maps);

  fourByteIntegerArrays.push_back (PlanFunction::CPP::accesses);

  fourByteIntegerArrays.push_back (PlanFunction::CPP::inds);

  for (vector <string>::iterator it = fourByteIntegerArrays.begin (); it
      != fourByteIntegerArrays.end (); ++it)
  {
    variableDeclarations->add (*it,
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            CPPTypesBuilder::getArray_RankOne (
                CPPTypesBuilder::getFourByteInteger (), 1,
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
          CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, CPPTypesBuilder::getArray_RankOne (
                  CPPTypesBuilder::getTwoByteInteger ()), subroutineScope,
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
          CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, CPPTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of integer fields. These fields need to be accessed
   * on the CPP side, so create local variables that
   * enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> fourByteIntegerVariables;

  fourByteIntegerVariables.push_back (CommonVariableNames::col);

  fourByteIntegerVariables.push_back (CommonVariableNames::iterationCounter1);

  fourByteIntegerVariables.push_back (PlanFunction::CPP::argsNumber);

  fourByteIntegerVariables.push_back (PlanFunction::CPP::indsNumber);

  fourByteIntegerVariables.push_back (PlanFunction::CPP::blockOffset);

  fourByteIntegerVariables.push_back (PlanFunction::CPP::pindSizesSize);

  fourByteIntegerVariables.push_back (PlanFunction::CPP::pindOffsSize);

  fourByteIntegerVariables.push_back (PlanFunction::CPP::pblkMapSize);

  fourByteIntegerVariables.push_back (PlanFunction::CPP::poffsetSize);

  fourByteIntegerVariables.push_back (PlanFunction::CPP::pnelemsSize);

  fourByteIntegerVariables.push_back (PlanFunction::CPP::pnthrcolSize);

  fourByteIntegerVariables.push_back (PlanFunction::CPP::pthrcolSize);

  for (vector <string>::iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            CPPTypesBuilder::getFourByteInteger (), subroutineScope));
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of integer array fields. These fields need to be
   * accessed on the CPP side ON THE HOST, so create local
   * variables that enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> integerPointerVariables;

  integerPointerVariables.push_back (PlanFunction::CPP::ncolblk);

  integerPointerVariables.push_back (PlanFunction::CPP::pnindirect);

  for (vector <string>::iterator it = integerPointerVariables.begin (); it
      != integerPointerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildPointerType (CPPTypesBuilder::getArray_RankOne (
                CPPTypesBuilder::getFourByteInteger ())), subroutineScope));
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of array integer fields. These fields need to be accessed
   * on the CPP side ON THE DEVICE, so create local variables
   * that enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> deviceIntegerArrayVariables;

  deviceIntegerArrayVariables.push_back (PlanFunction::CPP::pindSizes);

  deviceIntegerArrayVariables.push_back (PlanFunction::CPP::pindOffs);

  deviceIntegerArrayVariables.push_back (PlanFunction::CPP::pblkMap);

  deviceIntegerArrayVariables.push_back (PlanFunction::CPP::poffset);

  deviceIntegerArrayVariables.push_back (PlanFunction::CPP::pnelems);

  deviceIntegerArrayVariables.push_back (PlanFunction::CPP::pnthrcol);

  deviceIntegerArrayVariables.push_back (PlanFunction::CPP::pthrcol);

  for (vector <string>::iterator it = deviceIntegerArrayVariables.begin (); it
      != deviceIntegerArrayVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            CPPTypesBuilder::getArray_RankOne (
                CPPTypesBuilder::getFourByteInteger ()), subroutineScope,
            2, DEVICE, ALLOCATABLE));
  }
}

void
CPPOpenCLHostSubroutineIndirectLoop::createStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildBoolValExp;
  using SageBuilder::buildEqualityOp;
  using SageInterface::appendStatement;
  using std::vector;

  vector <SubroutineVariableDeclarations *> declarationSets;
  declarationSets.push_back (moduleDeclarations->getAllDeclarations ());
  declarationSets.push_back (variableDeclarations);

  SubroutineVariableDeclarations * allDeclarations =
      new SubroutineVariableDeclarations (declarationSets);

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (true));

  SgBasicBlock * ifBody = createFirstTimeExecutionStatements ();

  appendStatement (createPlanFunctionParametersPreparationStatements (
      allDeclarations, (CPPParallelLoop *) parallelLoop), ifBody);

  appendStatement (createPlanFunctionCallStatement (allDeclarations,
      subroutineScope), ifBody);

  appendStatement (createInitialiseConstantsCallStatement (), ifBody);

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, subroutineScope);

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionPrologueStatements ();
  }

  appendStatement (createConvertPositionInPMapsStatements (allDeclarations,
      (CPPParallelLoop *) parallelLoop, subroutineScope), subroutineScope);

  appendStatement (createConvertPlanFunctionParametersStatements (
      allDeclarations, (CPPParallelLoop *) parallelLoop, subroutineScope),
      subroutineScope);

  createVariablesSizesInitialisationStatements ();

  createPlanFunctionExecutionStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPOpenCLHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  createDataMarshallingLocalVariableDeclarations ();

  createOpenCLKernelLocalVariableDeclarations ();

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

CPPOpenCLHostSubroutineIndirectLoop::CPPOpenCLHostSubroutineIndirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName,
    CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    CPPInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
    CPPOpenCLDataSizesDeclarationIndirectLoop * dataSizesDeclaration,
    CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
    CPPOpenCLModuleDeclarationsIndirectLoop * moduleDeclarations) :
  CPPOpenCLHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope,
      initialiseConstantsSubroutine, dataSizesDeclaration,
      opDatDimensionsDeclaration, moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
