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

namespace
{
  std::string const reductionOperation = "reductionOperation";
  std::string const inputValue = "inputValue";
}

void
FortranCUDAReductionSubroutine::createThreadZeroReductionStatements ()
{
  using SageBuilder::buildSwitchStatement;
  using SageBuilder::buildCaseOptionStmt;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildGreaterThanOp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * The increment reduction case
   * ======================================================
   */

  SgPntrArrRefExp * subscriptExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::reductionResult)),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression2 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::reductionResult)),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression3 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), buildIntVal (0));

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

  SgPntrArrRefExp * subscriptExpression4 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::reductionResult)),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression5 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), buildIntVal (0));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      subscriptExpression4, subscriptExpression5);

  SgPntrArrRefExp * subscriptExpression6 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::reductionResult)),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression7 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), buildIntVal (0));

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

  SgPntrArrRefExp * subscriptExpression8 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::reductionResult)),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression9 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), buildIntVal (0));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      subscriptExpression8, subscriptExpression9);

  SgPntrArrRefExp * subscriptExpression10 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::reductionResult)),
      buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression11 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), buildIntVal (0));

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

  SgSwitchStatement * switchStatement = buildSwitchStatement (buildVarRefExp (
      variableDeclarations->get (reductionOperation)), switchBody);

  /*
   * ======================================================
   * If statement determining whether this is thread 0
   * ======================================================
   */

  SgBasicBlock * ifBody = buildBasicBlock ();

  appendStatement (switchStatement, ifBody);

  SgExpression * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)),
      buildIntVal (0));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, subroutineScope);
}

void
FortranCUDAReductionSubroutine::createReductionStatements ()
{
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildGreaterThanOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildSwitchStatement;
  using SageBuilder::buildCaseOptionStmt;
  using SageBuilder::buildWhileStmt;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * The increment reduction case
   * ======================================================
   */
  SgPntrArrRefExp * arrayIndexExpressionA1 =
      buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
          autosharedVariableName)), buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::threadID)));

  SgAddOp * addExpressionA1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)),
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)));

  SgPntrArrRefExp * arrayIndexExpressionA2 = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (autosharedVariableName)),
      addExpressionA1);

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

  SgPntrArrRefExp * arrayIndexExpressionB1 =
      buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
          autosharedVariableName)), buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::threadID)));

  SgAddOp * addExpressionB1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)),
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)));

  SgPntrArrRefExp * arrayIndexExpressionB2 = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (autosharedVariableName)),
      addExpressionB1);

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

  SgPntrArrRefExp * arrayIndexExpressionC1 =
      buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
          autosharedVariableName)), buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::threadID)));

  SgAddOp * addExpressionC1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)),
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)));

  SgPntrArrRefExp * arrayIndexExpressionC2 = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (autosharedVariableName)),
      addExpressionC1);

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

  SgSwitchStatement * switchStatement = buildSwitchStatement (buildVarRefExp (
      variableDeclarations->get (reductionOperation)), switchBody);

  /*
   * ======================================================
   * If statement controlling whether to index array, depending
   * on whether the thread ID is less than the loop counter
   * ======================================================
   */

  SgBasicBlock * ifBlock = buildBasicBlock (switchStatement);

  SgExpression * ifGuardExpression = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)),
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  /*
   * ======================================================
   * Build the loop
   * ======================================================
   */

  SgExprListExp * actualParametersItCountReassign = buildExprListExp (
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)), buildIntVal (-1));

  SgFunctionSymbol * shiftFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("ishft", subroutineScope);

  SgFunctionCallExp * shiftFunctionCall = buildFunctionCallExp (
      shiftFunctionSymbol, actualParametersItCountReassign);

  SgExprStatement * reassignIterationCounter = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)), shiftFunctionCall);

  SgBasicBlock * whileLoopBody = buildBasicBlock ();

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      whileLoopBody);

  appendStatement (ifStatement, whileLoopBody);

  appendStatement (reassignIterationCounter, whileLoopBody);

  SgExpression * upperBoundExpression = buildGreaterThanOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildIntVal (0));

  SgWhileStmt * whileLoopStatement = buildWhileStmt (upperBoundExpression,
      whileLoopBody);

  whileLoopStatement->set_has_end_statement (true);

  appendStatement (whileLoopStatement, subroutineScope);
}

void
FortranCUDAReductionSubroutine::createSharedVariableInitialisationStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating shared variable initialisation statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgPntrArrRefExp * subscriptExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)));

  SgExprStatement * assignStatement1 = buildAssignStatement (
      subscriptExpression1, buildVarRefExp (variableDeclarations->get (
          inputValue)));

  appendStatement (assignStatement1, subroutineScope);
}

void
FortranCUDAReductionSubroutine::createInitialisationStatements ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating initialisation statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Initialise the thread ID
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgExprStatement * assignStatement1 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)),
      subtractExpression1);

  appendStatement (assignStatement1, subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp (
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope), buildIntVal (
          -1));

  SgFunctionSymbol * shiftFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("ishft", subroutineScope);

  SgFunctionCallExp * shiftFunctionCall = buildFunctionCallExp (
      shiftFunctionSymbol, actualParameters);

  SgExprStatement * assignStatement2 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      shiftFunctionCall);

  appendStatement (assignStatement2, subroutineScope);
}

void
FortranCUDAReductionSubroutine::createStatements ()
{
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;

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
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Create autoshared variable declaration with the same
   * base type and size as the reduction variable
   * ======================================================
   */

  autosharedVariableName = OP2::VariableNames::getAutosharedDeclarationName (
      reduction->getBaseType (), reduction->getVariableSize ());

  SgArrayType * arrayType = FortranTypesBuilder::getArray_RankOne (
      reduction->getBaseType (), 0, new SgAsteriskShapeExp (
          RoseHelper::getFileInfo ()));

  variableDeclarations->add (autosharedVariableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          autosharedVariableName, arrayType, subroutineScope, 1, SHARED));

  /*
   * ======================================================
   * Create thread ID and iteration counter variables
   * ======================================================
   */

  vector <string> fourByteIntegers;

  fourByteIntegers.push_back (CommonVariableNames::iterationCounter1);

  fourByteIntegers.push_back (OP2::VariableNames::threadID);

  for (vector <string>::iterator it = fourByteIntegers.begin (); it
      != fourByteIntegers.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }
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
          subroutineScope, formalParameters, 1, DEVICE));

  /*
   * ======================================================
   * Declare the value of the reduction variable produced by
   * each thread which is passed by value
   * ======================================================
   */

  variableDeclarations->add (
      inputValue,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          inputValue, reduction->getBaseType (), subroutineScope,
          formalParameters, 1, VALUE));

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
      reductionOperation,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          reductionOperation, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters, 1, VALUE));
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranCUDAReductionSubroutine::FortranCUDAReductionSubroutine (
    std::string const & subroutineAndVariableName,
    SgScopeStatement * moduleScope, Reduction * reduction) :
  Subroutine <SgProcedureHeaderStatement> (subroutineAndVariableName),
      reduction (reduction)
{
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionParameterList;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  Debug::getInstance ()->debugMessage ("Creating reduction subroutine",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(device) ",
      AstUnparseAttribute::e_before);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
