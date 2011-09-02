#include <boost/lexical_cast.hpp>
#include <RoseHelper.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <FortranCUDAReductionSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

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

  SgPntrArrRefExp * subscriptExpression1 = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::reductionResultOnDevice)), buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression2 = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::reductionResultOnDevice)), buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression3 =
      buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
          autosharedVariableName)), buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      subscriptExpression1, buildAddOp (subscriptExpression2,
          subscriptExpression3));

  SgIntVal * incrementCaseExpression = buildIntVal (INCREMENT);

  SgCaseOptionStmt * incrementCaseStatement = buildCaseOptionStmt (
      incrementCaseExpression, assignmentStatement1);

  /*
   * ======================================================
   * The minimum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * subscriptExpression4 = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::reductionResultOnDevice)), buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression5 =
      buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
          autosharedVariableName)), buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      subscriptExpression4, subscriptExpression5);

  SgPntrArrRefExp * subscriptExpression6 = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::reductionResultOnDevice)), buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression7 =
      buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
          autosharedVariableName)), buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)));

  SgLessThanOp * minimumGuardExpression = buildLessThanOp (
      subscriptExpression7, subscriptExpression6);

  SgBasicBlock * minimumBody = buildBasicBlock (assignmentStatement2);

  SgIfStmt * minimumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          minimumGuardExpression, minimumBody);

  SgIntVal * minimumCaseExpression = buildIntVal (MINIMUM);

  SgCaseOptionStmt * minimumCaseStatement = buildCaseOptionStmt (
      minimumCaseExpression, minimumIfStatement);

  /*
   * ======================================================
   * The maximum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * subscriptExpression8 = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::reductionResultOnDevice)), buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression9 =
      buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
          autosharedVariableName)), buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      subscriptExpression8, subscriptExpression9);

  SgPntrArrRefExp * subscriptExpression10 = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::reductionResultOnDevice)), buildIntVal (1));

  SgPntrArrRefExp * subscriptExpression11 =
      buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
          autosharedVariableName)), buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)));

  SgGreaterThanOp * maximumGuardExpression = buildGreaterThanOp (
      subscriptExpression11, subscriptExpression10);

  SgBasicBlock * maximumBody = buildBasicBlock (assignmentStatement3);

  SgIfStmt * maximumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          maximumGuardExpression, maximumBody);

  SgIntVal * maximumCaseExpression = buildIntVal (MAXIMUM);

  SgCaseOptionStmt * maximumCaseStatement = buildCaseOptionStmt (
      maximumCaseExpression, maximumIfStatement);

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
      variableDeclarations->get (ReductionSubroutine::reductionOperation)),
      switchBody);

  /*
   * ======================================================
   * If statement determining whether this is thread 0
   * ======================================================
   */

  SgBasicBlock * ifBody = buildBasicBlock ();

  appendStatement (switchStatement, ifBody);

  SgExpression * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      variableDeclarations->get (ReductionSubroutine::threadID)), buildIntVal (
      0));

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
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * The increment reduction case
   * ======================================================
   */

  SgAddOp * addExpression1 = buildAddOp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)), buildVarRefExp (
          variableDeclarations->get (ReductionSubroutine::threadID)));

  SgAddOp * addExpression2 = buildAddOp (addExpression1, buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)));

  SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression1);

  SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression2);

  SgAddOp * addExpression3 = buildAddOp (arrayIndexExpression1,
      arrayIndexExpression2);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayIndexExpression1, addExpression3);

  SgIntVal * incrementCaseExpression = buildIntVal (INCREMENT);

  SgCaseOptionStmt * incrementCaseStatement = buildCaseOptionStmt (
      incrementCaseExpression, assignmentStatement1);

  /*
   * ======================================================
   * The minimum reduction case
   * ======================================================
   */

  SgAddOp * addExpression4 = buildAddOp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)), buildVarRefExp (
          variableDeclarations->get (ReductionSubroutine::threadID)));

  SgAddOp * addExpression5 = buildAddOp (addExpression4, buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)));

  SgPntrArrRefExp * arrayIndexExpression3 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression4);

  SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression5);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      arrayIndexExpression3, arrayIndexExpression4);

  SgLessThanOp * minimumGuardExpression = buildLessThanOp (
      arrayIndexExpression4, arrayIndexExpression3);

  SgBasicBlock * minimumBody = buildBasicBlock (assignmentStatement2);

  SgIfStmt * minimumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          minimumGuardExpression, minimumBody);

  SgIntVal * minimumCaseExpression = buildIntVal (MINIMUM);

  SgCaseOptionStmt * minimumCaseStatement = buildCaseOptionStmt (
      minimumCaseExpression, minimumIfStatement);

  /*
   * ======================================================
   * The maximum reduction case
   * ======================================================
   */

  SgAddOp * addExpression6 = buildAddOp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)), buildVarRefExp (
          variableDeclarations->get (ReductionSubroutine::threadID)));

  SgAddOp * addExpression7 = buildAddOp (addExpression6, buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)));

  SgPntrArrRefExp * arrayIndexExpression5 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression6);

  SgPntrArrRefExp * arrayIndexExpression6 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression7);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      arrayIndexExpression5, arrayIndexExpression6);

  SgGreaterThanOp * maximumGuardExpression = buildGreaterThanOp (
      arrayIndexExpression6, arrayIndexExpression5);

  SgBasicBlock * maximumBody = buildBasicBlock (assignmentStatement3);

  SgIfStmt * maximumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          maximumGuardExpression, maximumBody);

  SgIntVal * maximumCaseExpression = buildIntVal (MAXIMUM);

  SgCaseOptionStmt * maximumCaseStatement = buildCaseOptionStmt (
      maximumCaseExpression, maximumIfStatement);

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
      variableDeclarations->get (ReductionSubroutine::reductionOperation)),
      switchBody);

  /*
   * ======================================================
   * If statement controlling whether to index array, depending
   * on whether the thread ID is less than the loop counter
   * ======================================================
   */

  SgBasicBlock * ifBlock = buildBasicBlock (switchStatement);

  SgExpression * ifGuardExpression = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (ReductionSubroutine::threadID)),
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

  appendStatement (createSynchThreadsCallStatement (), whileLoopBody);

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
  using SageBuilder::buildAddOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  SgAddOp * arrayIndexExpression1 = buildAddOp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)), buildVarRefExp (
          variableDeclarations->get (ReductionSubroutine::threadID)));

  SgPntrArrRefExp * subscriptExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)),
      arrayIndexExpression1);

  SgExprStatement * assignStatement1 = buildAssignStatement (
      subscriptExpression1, buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::inputValue)));

  appendStatement (assignStatement1, subroutineScope);
}

void
FortranCUDAReductionSubroutine::createInitialisationStatements ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * Initialise the thread ID
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (buildDotExp (
      buildOpaqueVarRefExp (CUDA::Fortran::threadidx, subroutineScope),
      buildOpaqueVarRefExp (CUDA::Fortran::x, subroutineScope)),
      buildIntVal (1));

  SgExprStatement * assignStatement1 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (ReductionSubroutine::threadID)),
      subtractExpression1);

  appendStatement (assignStatement1, subroutineScope);

  /*
   * ======================================================
   * Initialise the displacement variable to blockdim%x
   * right shifted one place
   * ======================================================
   */

  SgDotExp * dotExpression2 = buildDotExp (buildOpaqueVarRefExp (
      CUDA::Fortran::blockdim, subroutineScope), buildOpaqueVarRefExp (
      CUDA::Fortran::x, subroutineScope));

  SgExprListExp * actualParameters = buildExprListExp (dotExpression2,
      buildIntVal (-1));

  SgFunctionSymbol * shiftFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("ishft", subroutineScope);

  SgFunctionCallExp * shiftFunctionCall = buildFunctionCallExp (
      shiftFunctionSymbol, actualParameters);

  SgExprStatement * assignStatement2 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      shiftFunctionCall);

  appendStatement (assignStatement2, subroutineScope);

  /*
   * ======================================================
   * Initialise the shared memory offset to zero
   * ======================================================
   */

  SgExprStatement * assignStatement3 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)), buildIntVal (0));

  appendStatement (assignStatement3, subroutineScope);
}

SgStatement *
FortranCUDAReductionSubroutine::createSynchThreadsCallStatement ()
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildExprStatement;

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine (
          CUDA::Fortran::syncthreads, subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp ();

  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  return buildExprStatement (subroutineCall);
}

void
FortranCUDAReductionSubroutine::createStatements ()
{
  using SageInterface::appendStatement;

  createInitialisationStatements ();

  appendStatement (createSynchThreadsCallStatement (), subroutineScope);

  createSharedVariableInitialisationStatements ();

  createReductionStatements ();

  appendStatement (createSynchThreadsCallStatement (), subroutineScope);

  createThreadZeroReductionStatements ();

  appendStatement (createSynchThreadsCallStatement (), subroutineScope);
}

void
FortranCUDAReductionSubroutine::createLocalVariableDeclarations ()
{
  using std::vector;
  using std::string;

  /*
   * ======================================================
   * Create autoshared variable declaration with the same
   * base type and size as the reduction variable
   * ======================================================
   */

  autosharedVariableName = VariableNames::getAutosharedDeclarationName (
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

  fourByteIntegers.push_back (ReductionSubroutine::threadID);

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
      ReductionSubroutine::reductionResultOnDevice,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::reductionResultOnDevice,
          reduction->getArrayType (), subroutineScope, formalParameters, 1,
          DEVICE));

  /*
   * ======================================================
   * Declare the value of the reduction variable produced by
   * each thread which is passed by value
   * ======================================================
   */

  variableDeclarations->add (
      ReductionSubroutine::inputValue,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::inputValue, reduction->getBaseType (),
          subroutineScope, formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Declare the the warp size: currently, in CUDA Fortran
   * it is not possible to access the warpsize variable value
   * in a device subroutine (only possible in global ones)
   * ======================================================
   */

  variableDeclarations->add (
      ReductionSubroutine::warpSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::warpSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Declare the the offset from which the automatically allocated
   * shared memory variable space is reserved for use by
   * the reduction subroutine
   * ======================================================
   */

  variableDeclarations->add (
      ReductionSubroutine::sharedMemoryStartOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::sharedMemoryStartOffset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Declare the reduction operation variable
   * ======================================================
   */

  variableDeclarations->add (
      ReductionSubroutine::reductionOperation,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::reductionOperation,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));
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
