#include <FortranCUDAReductionSubroutine.h>
#include <Reduction.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CUDA.h>
#include <boost/lexical_cast.hpp>

void
FortranCUDAReductionSubroutine::createThreadZeroReductionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  /*
   * ======================================================
   * The increment reduction case
   * ======================================================
   */

  SgPntrArrRefExp * subscriptExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (OP2::VariableNames::reductionResult),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (OP2::VariableNames::reductionResult),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression3 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), buildIntVal (0));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      subscriptExpression1, buildAddOp (subscriptExpression2,
          subscriptExpression3));

  SgIntVal * incrementCaseExpression = buildIntVal (INCREMENT);

  SgCaseOptionStmt * incrementCaseStatement = buildCaseOptionStmt (
      incrementCaseExpression, buildBasicBlock (assignmentStatement1));

  /*
   * ======================================================
   * The minimum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * subscriptExpression4 = buildPntrArrRefExp (
      variableDeclarations->getReference (OP2::VariableNames::reductionResult),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression5 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), buildIntVal (0));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      subscriptExpression4, subscriptExpression5);

  SgPntrArrRefExp * subscriptExpression6 = buildPntrArrRefExp (
      variableDeclarations->getReference (OP2::VariableNames::reductionResult),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression7 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), buildIntVal (0));

  SgLessThanOp * minimumGuardExpression = buildLessThanOp (
      subscriptExpression7, subscriptExpression6);

  SgBasicBlock * minimumBody = buildBasicBlock (assignmentStatement2);

  SgIfStmt * minimumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          minimumGuardExpression, minimumBody);

  SgIntVal * minimumCaseExpression = buildIntVal (MINIMUM);

  SgCaseOptionStmt * minimumCaseStatement = buildCaseOptionStmt (
      minimumCaseExpression, buildBasicBlock (minimumIfStatement));

  /*
   * ======================================================
   * The maximum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * subscriptExpression8 = buildPntrArrRefExp (
      variableDeclarations->getReference (OP2::VariableNames::reductionResult),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression9 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), buildIntVal (0));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      subscriptExpression8, subscriptExpression9);

  SgPntrArrRefExp * subscriptExpression10 = buildPntrArrRefExp (
      variableDeclarations->getReference (OP2::VariableNames::reductionResult),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression11 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), buildIntVal (0));

  SgGreaterThanOp * maximumGuardExpression = buildGreaterThanOp (
      subscriptExpression11, subscriptExpression10);

  SgBasicBlock * maximumBody = buildBasicBlock (assignmentStatement3);

  SgIfStmt * maximumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          maximumGuardExpression, maximumBody);

  SgIntVal * maximumCaseExpression = buildIntVal (MAXIMUM);

  SgCaseOptionStmt * maximumCaseStatement = buildCaseOptionStmt (
      maximumCaseExpression, buildBasicBlock (maximumIfStatement));

  /*
   * ======================================================
   * Switch statement on the reduction type with corresponding
   * cases
   * ======================================================
   */

  SgBasicBlock * switchBody = buildBasicBlock ();

  appendStatement (incrementCaseStatement, switchBody);

  appendStatement (minimumCaseStatement, switchBody);

  appendStatement (maximumCaseStatement, switchBody);

  SgSwitchStatement * switchStatement = buildSwitchStatement (
      variableDeclarations->getReference (
          OP2::VariableNames::reductionOperation), switchBody);

  /*
   * ======================================================
   * If statement determining whether this is thread 0
   * ======================================================
   */

  SgBasicBlock * ifBody = buildBasicBlock ();

  appendStatement (switchStatement, ifBody);

  SgExpression * ifGuardExpression = buildEqualityOp (
      variableDeclarations->getReference (OP2::VariableNames::threadID),
      buildIntVal (0));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, subroutineScope);
}

void
FortranCUDAReductionSubroutine::createReductionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  /*
   * ======================================================
   * The increment reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayIndexExpressionA1 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (OP2::VariableNames::threadID));

  SgAddOp * addExpressionA1 = buildAddOp (variableDeclarations->getReference (
      OP2::VariableNames::threadID), variableDeclarations->getReference (
      CommonVariableNames::iterationCounter1));

  SgPntrArrRefExp * arrayIndexExpressionA2 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), addExpressionA1);

  SgAddOp * addExpression3 = buildAddOp (arrayIndexExpressionA1,
      arrayIndexExpressionA2);

  SgExprStatement * assignmentStatementA = buildAssignStatement (
      arrayIndexExpressionA1, addExpression3);

  SgIntVal * incrementCaseExpression = buildIntVal (INCREMENT);

  SgCaseOptionStmt * incrementCaseStatement = buildCaseOptionStmt (
      incrementCaseExpression, buildBasicBlock (assignmentStatementA));

  /*
   * ======================================================
   * The minimum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayIndexExpressionB1 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (CommonVariableNames::threadID));

  SgAddOp * addExpressionB1 = buildAddOp (variableDeclarations->getReference (
      OP2::VariableNames::threadID), variableDeclarations->getReference (
      CommonVariableNames::iterationCounter1));

  SgPntrArrRefExp * arrayIndexExpressionB2 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), addExpressionB1);

  SgExprStatement * assignmentStatementB = buildAssignStatement (
      arrayIndexExpressionB1, arrayIndexExpressionB2);

  SgLessThanOp * minimumGuardExpression = buildLessThanOp (
      arrayIndexExpressionB2, arrayIndexExpressionB1);

  SgBasicBlock * minimumBody = buildBasicBlock (assignmentStatementB);

  SgIfStmt * minimumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          minimumGuardExpression, minimumBody);

  SgIntVal * minimumCaseExpression = buildIntVal (MINIMUM);

  SgCaseOptionStmt * minimumCaseStatement = buildCaseOptionStmt (
      minimumCaseExpression, buildBasicBlock (minimumIfStatement));

  /*
   * ======================================================
   * The maximum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayIndexExpressionC1 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (CommonVariableNames::threadID));

  SgAddOp * addExpressionC1 = buildAddOp (variableDeclarations->getReference (
      OP2::VariableNames::threadID), variableDeclarations->getReference (
      CommonVariableNames::iterationCounter1));

  SgPntrArrRefExp * arrayIndexExpressionC2 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), addExpressionC1);

  SgExprStatement * assignmentStatementC = buildAssignStatement (
      arrayIndexExpressionC1, arrayIndexExpressionC2);

  SgGreaterThanOp * maximumGuardExpression = buildGreaterThanOp (
      arrayIndexExpressionC2, arrayIndexExpressionC1);

  SgBasicBlock * maximumBody = buildBasicBlock (assignmentStatementC);

  SgIfStmt * maximumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          maximumGuardExpression, maximumBody);

  SgIntVal * maximumCaseExpression = buildIntVal (MAXIMUM);

  SgCaseOptionStmt * maximumCaseStatement = buildCaseOptionStmt (
      maximumCaseExpression, buildBasicBlock (maximumIfStatement));

  /*
   * ======================================================
   * Switch statement on the reduction type with corresponding
   * cases
   * ======================================================
   */

  SgBasicBlock * switchBody = buildBasicBlock ();

  appendStatement (incrementCaseStatement, switchBody);

  appendStatement (minimumCaseStatement, switchBody);

  appendStatement (maximumCaseStatement, switchBody);

  SgSwitchStatement * switchStatement = buildSwitchStatement (
      variableDeclarations->getReference (
          OP2::VariableNames::reductionOperation), switchBody);

  /*
   * ======================================================
   * If statement controlling whether to index array, depending
   * on whether the thread ID is less than the loop counter
   * ======================================================
   */

  SgBasicBlock * ifBlock = buildBasicBlock (switchStatement);

  SgExpression * ifGuardExpression = buildLessThanOp (
      variableDeclarations->getReference (OP2::VariableNames::threadID),
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  /*
   * ======================================================
   * Build the loop
   * ======================================================
   */

  SgExprListExp * actualParametersItCountReassign = buildExprListExp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1), buildIntVal (-1));

  SgFunctionSymbol * shiftFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("ishft", subroutineScope);

  SgFunctionCallExp * shiftFunctionCall = buildFunctionCallExp (
      shiftFunctionSymbol, actualParametersItCountReassign);

  SgExprStatement * reassignIterationCounter = buildAssignStatement (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1), shiftFunctionCall);

  SgBasicBlock * whileLoopBody = buildBasicBlock ();

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      whileLoopBody);

  appendStatement (ifStatement, whileLoopBody);

  appendStatement (reassignIterationCounter, whileLoopBody);

  SgExpression * upperBoundExpression = buildGreaterThanOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1), buildIntVal (0));

  SgWhileStmt * whileLoopStatement = buildWhileStmt (upperBoundExpression,
      whileLoopBody);

  whileLoopStatement->set_has_end_statement (true);

  appendStatement (whileLoopStatement, subroutineScope);
}

void
FortranCUDAReductionSubroutine::createSharedVariableInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating shared variable initialisation statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgPntrArrRefExp * subscriptExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (OP2::VariableNames::threadID));

  SgExprStatement * assignStatement1 = buildAssignStatement (
      subscriptExpression1, variableDeclarations->getReference (
          OP2::VariableNames::reductionInput));

  appendStatement (assignStatement1, subroutineScope);
}

void
FortranCUDAReductionSubroutine::createInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating initialisation statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Initialise the thread ID
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgExprStatement * assignStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OP2::VariableNames::threadID),
      subtractExpression1);

  appendStatement (assignStatement1, subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp (
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope), buildIntVal (
          -1));

  SgFunctionSymbol * shiftFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("ishft", subroutineScope);

  SgFunctionCallExp * shiftFunctionCall = buildFunctionCallExp (
      shiftFunctionSymbol, actualParameters);

  SgExprStatement * assignStatement2 = buildAssignStatement (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1), shiftFunctionCall);

  appendStatement (assignStatement2, subroutineScope);
}

void
FortranCUDAReductionSubroutine::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createInitialisationStatements ();

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  createSharedVariableInitialisationStatements ();

  createReductionStatements ();

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  createThreadZeroReductionStatements ();

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);
}

void
FortranCUDAReductionSubroutine::createLocalVariableDeclarations ()
{
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Create shared memory variable declaration with the same
   * base type and size as the reduction variable
   * ======================================================
   */

  sharedVariableName = OP2::VariableNames::getCUDASharedMemoryDeclarationName (
      reduction->getBaseType (), reduction->getVariableSize ());

  SgArrayType * arrayType = FortranTypesBuilder::getArray_RankOne (
      reduction->getBaseType (), 0, new SgAsteriskShapeExp (
          RoseHelper::getFileInfo ()));

  variableDeclarations->add (sharedVariableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          sharedVariableName, arrayType, subroutineScope, 1, CUDA_SHARED));

  /*
   * ======================================================
   * Create thread ID and iteration counter variables
   * ======================================================
   */

  variableDeclarations->add (CommonVariableNames::iterationCounter1,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::iterationCounter1,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (OP2::VariableNames::threadID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::threadID,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranCUDAReductionSubroutine::createFormalParameterDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating reduction procedure formal parameter", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Declare the device variable on which the result of local
   * reductions is stored by the first thread in the block
   * ======================================================
   */
  variableDeclarations->add (
      OP2::VariableNames::reductionResult,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::reductionResult,
          FortranTypesBuilder::getArray_RankOne (reduction->getBaseType ()),
          subroutineScope, formalParameters, 1, CUDA_DEVICE));

  /*
   * ======================================================
   * Declare the value of the reduction variable produced by
   * each thread which is passed by value
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::reductionInput,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::reductionInput, reduction->getBaseType (),
          subroutineScope, formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Declare the the warp size: currently, in CUDA Fortran
   * it is not possible to access the warpsize variable value
   * in a device subroutine (only possible in global ones)
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::warpSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::warpSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Declare the reduction operation variable
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::reductionOperation,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::reductionOperation,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));
}

FortranCUDAReductionSubroutine::FortranCUDAReductionSubroutine (
    SgScopeStatement * moduleScope, Reduction * reduction) :
  Subroutine <SgProcedureHeaderStatement> (reduction->getSubroutineName ()),
      reduction (reduction)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating reduction subroutine",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  subroutineHeaderStatement->get_functionModifier ().setCudaDevice ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
