#include <CPPCUDAReductionSubroutine.h>
#include <CUDA.h>
#include <Reduction.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CompilerGeneratedNames.h>

void
CPPCUDAReductionSubroutine::createThreadZeroReductionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;

  /*
   * ======================================================
   * The increment reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      buildIntVal (0));

  SgPointerDerefExp * pointerDerefExpression1 = buildPointerDerefExp (
      variableDeclarations->getReference (reductionResult));

  SgAddOp * addExpression = buildAddOp (pointerDerefExpression1,
      arrayExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildPointerDerefExp (
          variableDeclarations->getReference (reductionResult)), addExpression);

  SgIntVal * incrementCaseExpression = buildIntVal (INCREMENT);

  SgBasicBlock * incrementBasicBlock = buildBasicBlock ();

  appendStatement (assignmentStatement1, incrementBasicBlock);

  appendStatement (buildBreakStmt (), incrementBasicBlock);

  SgCaseOptionStmt * incrementCaseStatement = buildCaseOptionStmt (
      incrementCaseExpression, incrementBasicBlock);

  /*
   * ======================================================
   * The maximum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      buildIntVal (0));

  SgPointerDerefExp * pointerDerefExpression2 = buildPointerDerefExp (
      variableDeclarations->getReference (reductionResult));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      pointerDerefExpression2, arrayExpression2);

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), buildIntVal (0));

  SgGreaterThanOp * ifGuardExpression1 = buildGreaterThanOp (arrayExpression3,
      pointerDerefExpression2);

  SgIfStmt * maximumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression1, buildBasicBlock (assignmentStatement2));

  SgBasicBlock * maximumBasicBlock = buildBasicBlock ();

  appendStatement (maximumIfStatement, maximumBasicBlock);

  appendStatement (buildBreakStmt (), maximumBasicBlock);

  SgIntVal * maximumCaseExpression = buildIntVal (MAXIMUM);

  SgCaseOptionStmt * maximumCaseStatement = buildCaseOptionStmt (
      maximumCaseExpression, maximumBasicBlock);

  /*
   * ======================================================
   * The minimum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      buildIntVal (0));

  SgPointerDerefExp * pointerDerefExpression3 = buildPointerDerefExp (
      variableDeclarations->getReference (reductionResult));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      pointerDerefExpression3, arrayExpression4);

  SgPntrArrRefExp * arrayExpression5 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), buildIntVal (0));

  SgLessThanOp * ifGuardExpression2 = buildLessThanOp (arrayExpression5,
      pointerDerefExpression3);

  SgIfStmt * minimumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression2, buildBasicBlock (assignmentStatement3));

  SgBasicBlock * minimumBasicBlock = buildBasicBlock ();

  appendStatement (minimumIfStatement, minimumBasicBlock);

  appendStatement (buildBreakStmt (), minimumBasicBlock);

  SgIntVal * minimumCaseExpression = buildIntVal (MINIMUM);

  SgCaseOptionStmt * minimumCaseStatement = buildCaseOptionStmt (
      minimumCaseExpression, minimumBasicBlock);

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
      variableDeclarations->getReference (reductionOperation), switchBody);

  /*
   * ======================================================
   * If statement determining whether this is thread 0
   * ======================================================
   */

  SgBasicBlock * outerIfBody = buildBasicBlock ();

  appendStatement (switchStatement, outerIfBody);

  SgExpression * ifGuardExpression3 = buildEqualityOp (
      variableDeclarations->getReference (threadID), buildIntVal (0));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression3, outerIfBody);

  appendStatement (ifStatement, subroutineScope);
}

void
CPPCUDAReductionSubroutine::createSecondRoundOfReduceStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating second round of statements to perform reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * The increment reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      addExpression1);

  SgAddOp * addExpression2 = buildAddOp (arrayExpression1, arrayExpression2);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, addExpression2);

  SgIntVal * incrementCaseExpression = buildIntVal (INCREMENT);

  SgBasicBlock * incrementBasicBlock = buildBasicBlock ();

  appendStatement (assignmentStatement1, incrementBasicBlock);

  appendStatement (buildBreakStmt (), incrementBasicBlock);

  SgCaseOptionStmt * incrementCaseStatement = buildCaseOptionStmt (
      incrementCaseExpression, incrementBasicBlock);

  /*
   * ======================================================
   * The maximum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression3 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      addExpression3);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      arrayExpression3, arrayExpression4);

  SgGreaterThanOp * ifGuardExpression1 = buildGreaterThanOp (arrayExpression4,
      arrayExpression3);

  SgIfStmt * maximumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression1, buildBasicBlock (assignmentStatement2));

  SgBasicBlock * maximumBasicBlock = buildBasicBlock ();

  appendStatement (maximumIfStatement, maximumBasicBlock);

  appendStatement (buildBreakStmt (), maximumBasicBlock);

  SgIntVal * maximumCaseExpression = buildIntVal (MAXIMUM);

  SgCaseOptionStmt * maximumCaseStatement = buildCaseOptionStmt (
      maximumCaseExpression, maximumBasicBlock);

  /*
   * ======================================================
   * The minimum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression5 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression4 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression6 = buildPntrArrRefExp (
      variableDeclarations->getReference (volatileSharedVariableName),
      addExpression4);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      arrayExpression5, arrayExpression6);

  SgLessThanOp * ifGuardExpression2 = buildLessThanOp (arrayExpression6,
      arrayExpression5);

  SgIfStmt * minimumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression2, buildBasicBlock (assignmentStatement3));

  SgBasicBlock * minimumBasicBlock = buildBasicBlock ();

  appendStatement (minimumIfStatement, minimumBasicBlock);

  appendStatement (buildBreakStmt (), minimumBasicBlock);

  SgIntVal * minimumCaseExpression = buildIntVal (MINIMUM);

  SgCaseOptionStmt * minimumCaseStatement = buildCaseOptionStmt (
      minimumCaseExpression, minimumBasicBlock);

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
      variableDeclarations->getReference (reductionOperation), switchBody);

  /*
   * ======================================================
   * If statement
   * ======================================================
   */

  SgLessThanOp * ifGuardExpression3 = buildLessThanOp (
      variableDeclarations->getReference (threadID),
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgBasicBlock * ifBody = buildBasicBlock ();

  appendStatement (switchStatement, ifBody);

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression3, ifBody);

  /*
   * ======================================================
   * For loop statement
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (ifStatement, loopBody);

  SgGreaterThanOp * testExpression = buildGreaterThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgRshiftAssignOp * strideExpression = buildRshiftAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (1));

  SgForStatement * forStatement = buildForStatement (buildNullStatement (),
      buildExprStatement (testExpression), strideExpression, loopBody);

  /*
   * ======================================================
   * If statement
   * ======================================================
   */

  SgLessThanOp * ifGuardExpression4 = buildLessThanOp (
      variableDeclarations->getReference (threadID),
      CUDA::getWarpSizeReference (subroutineScope));

  SgBasicBlock * ifBody2 = buildBasicBlock ();

  appendStatement (forStatement, ifBody2);

  SgIfStmt * ifStatement2 =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression4, ifBody2);

  appendStatement (ifStatement2, subroutineScope);
}

void
CPPCUDAReductionSubroutine::createFirstRoundOfReduceStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating first round of statements to perform reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * The increment reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), addExpression1);

  SgAddOp * addExpression2 = buildAddOp (arrayExpression1, arrayExpression2);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, addExpression2);

  SgIntVal * incrementCaseExpression = buildIntVal (INCREMENT);

  SgBasicBlock * incrementBasicBlock = buildBasicBlock ();

  appendStatement (assignmentStatement1, incrementBasicBlock);

  appendStatement (buildBreakStmt (), incrementBasicBlock);

  SgCaseOptionStmt * incrementCaseStatement = buildCaseOptionStmt (
      incrementCaseExpression, incrementBasicBlock);

  /*
   * ======================================================
   * The maximum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression3 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), addExpression3);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      arrayExpression3, arrayExpression4);

  SgGreaterThanOp * ifGuardExpression1 = buildGreaterThanOp (arrayExpression4,
      arrayExpression3);

  SgIfStmt * maximumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression1, buildBasicBlock (assignmentStatement2));

  SgBasicBlock * maximumBasicBlock = buildBasicBlock ();

  appendStatement (maximumIfStatement, maximumBasicBlock);

  appendStatement (buildBreakStmt (), maximumBasicBlock);

  SgIntVal * maximumCaseExpression = buildIntVal (MAXIMUM);

  SgCaseOptionStmt * maximumCaseStatement = buildCaseOptionStmt (
      maximumCaseExpression, maximumBasicBlock);

  /*
   * ======================================================
   * The minimum reduction case
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression5 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (threadID));

  SgAddOp * addExpression4 = buildAddOp (variableDeclarations->getReference (
      threadID), variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  SgPntrArrRefExp * arrayExpression6 = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName), addExpression4);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      arrayExpression5, arrayExpression6);

  SgLessThanOp * ifGuardExpression2 = buildLessThanOp (arrayExpression6,
      arrayExpression5);

  SgIfStmt * minimumIfStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression2, buildBasicBlock (assignmentStatement3));

  SgBasicBlock * minimumBasicBlock = buildBasicBlock ();

  appendStatement (minimumIfStatement, minimumBasicBlock);

  appendStatement (buildBreakStmt (), minimumBasicBlock);

  SgIntVal * minimumCaseExpression = buildIntVal (MINIMUM);

  SgCaseOptionStmt * minimumCaseStatement = buildCaseOptionStmt (
      minimumCaseExpression, minimumBasicBlock);

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
      variableDeclarations->getReference (reductionOperation), switchBody);

  /*
   * ======================================================
   * If statement
   * ======================================================
   */

  SgLessThanOp * ifGuardExpression3 = buildLessThanOp (
      variableDeclarations->getReference (threadID),
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgBasicBlock * ifBody = buildBasicBlock ();

  appendStatement (switchStatement, ifBody);

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression3, ifBody);

  /*
   * ======================================================
   * For loop statement
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      loopBody);

  appendStatement (ifStatement, loopBody);

  SgGreaterThanOp * testExpression = buildGreaterThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      CUDA::getWarpSizeReference (subroutineScope));

  SgRshiftAssignOp * strideExpression = buildRshiftAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (1));

  SgForStatement * forStatement = buildForStatement (buildNullStatement (),
      buildExprStatement (testExpression), strideExpression, loopBody);

  appendStatement (forStatement, subroutineScope);
}

void
CPPCUDAReductionSubroutine::createSharedVariableInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating shared variable initialisation statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
      variableDeclarations->getReference (sharedVariableName),
      variableDeclarations->getReference (threadID));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression, variableDeclarations->getReference (reductionInput));

  appendStatement (assignmentStatement1, subroutineScope);
}

void
CPPCUDAReductionSubroutine::createInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (threadID), CUDA::getThreadId (
          THREAD_X, subroutineScope));

  appendStatement (assignmentStatement1, subroutineScope);

  SgRshiftOp * shiftExpression = buildRshiftOp (CUDA::getThreadBlockDimension (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      shiftExpression);

  appendStatement (assignmentStatement2, subroutineScope);
}

void
CPPCUDAReductionSubroutine::createStatements ()
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

  createFirstRoundOfReduceStatements ();

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (volatileSharedVariableName),
      variableDeclarations->getReference (sharedVariableName));

  appendStatement (assignmentStatement1, subroutineScope);

  createSecondRoundOfReduceStatements ();

  createThreadZeroReductionStatements ();
}

void
CPPCUDAReductionSubroutine::createLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Create shared memory variable declaration with the same
   * base type and size as the reduction variable
   * ======================================================
   */

  sharedVariableName = getCUDASharedMemoryDeclarationName (
      reduction->getBaseType (), reduction->getVariableSize ());

  SgVariableDeclaration * sharedVariableDeclaration =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          sharedVariableName, buildArrayType (reduction->getBaseType ()),
          subroutineScope);

  sharedVariableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaDynamicShared ();

  variableDeclarations->add (sharedVariableName, sharedVariableDeclaration);

  /*
   * ======================================================
   * We also need a shared memory pointer with the volatile
   * attribute to be compatible with Fermi
   * ======================================================
   */

  volatileSharedVariableName = getCUDAVolatileSharedMemoryDeclarationName (
      reduction->getBaseType (), reduction->getVariableSize ());

  variableDeclarations->add (volatileSharedVariableName,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          volatileSharedVariableName, buildVolatileType (buildPointerType (
              reduction->getBaseType ())), subroutineScope));

  /*
   * ======================================================
   * Create thread ID and iteration counter variables
   * ======================================================
   */

  variableDeclarations->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));

  variableDeclarations->add (threadID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (threadID,
          buildIntType (), subroutineScope));
}

void
CPPCUDAReductionSubroutine::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;

  variableDeclarations->add (
      reductionResult,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          reductionResult, buildVolatileType (buildPointerType (
              reduction->getBaseType ())), subroutineScope, formalParameters));

  variableDeclarations->add (
      reductionInput,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          reductionInput, reduction->getBaseType (), subroutineScope,
          formalParameters));

  /*
   * ======================================================
   * Declare the reduction operation variable
   * ======================================================
   */

  variableDeclarations->add (
      reductionOperation,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          reductionOperation, buildIntType (), subroutineScope,
          formalParameters));
}

CPPCUDAReductionSubroutine::CPPCUDAReductionSubroutine (
    SgScopeStatement * moduleScope, Reduction * reduction) :
  Subroutine <SgFunctionDeclaration> (reduction->getSubroutineName ()),
      reduction (reduction)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating reduction subroutine",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  subroutineHeaderStatement->get_functionModifier ().setCudaDevice ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
