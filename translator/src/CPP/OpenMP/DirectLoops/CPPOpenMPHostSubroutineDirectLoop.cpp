#include <CPPOpenMPHostSubroutineDirectLoop.h>
#include <CPPUserSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <CommonNamespaces.h>
#include <OpenMP.h>

SgStatement *
CPPOpenMPHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;

  Debug::getInstance ()->debugMessage ("Creating call to user kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDirect (i))
    {
      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
          getOpDatLocalName (i)), multiplyExpression);

      actualParameters->append_expression (addExpression);
    }
    else if (parallelLoop->isReductionRequired (i))
    {
      actualParameters->append_expression (variableDeclarations->getReference (
          getOpDatLocalName (i)));
    }
  }

  SgFunctionCallExp * functionCallExpression = buildFunctionCallExp (
      calleeSubroutine->getSubroutineName (), buildVoidType (),
      actualParameters, subroutineScope);

  return buildExprStatement (functionCallExpression);
}

SgForStatement *
CPPOpenMPHostSubroutineDirectLoop::createThreadLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;
  using namespace OpenMP;

  Debug::getInstance ()->debugMessage (
      "Creating for loop statements for thread slice", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  appendStatement (createKernelFunctionCallStatement (), loopBody);

  /*
   * ======================================================
   * For loop statement
   * ======================================================
   */

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      variableDeclarations->getReference (sliceStart));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      variableDeclarations->getReference (sliceEnd));

  SgPlusPlusOp * strideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)));

  SgForStatement * forLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  return forLoopStatement;
}

SgOmpParallelStatement *
CPPOpenMPHostSubroutineDirectLoop::createOpenMPLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;
  using namespace OpenMP;

  Debug::getInstance ()->debugMessage ("Creating OpenMP for loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgArrowExp * arrowExpression1 = buildArrowExp (
      variableDeclarations->getReference (set), buildOpaqueVarRefExp (size,
          subroutineScope));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrowExpression1,
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgDivideOp * divideExpression1 = buildDivideOp (multiplyExpression1,
      variableDeclarations->getReference (numberOfThreads));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (sliceStart), divideExpression1);

  appendStatement (assignmentStatement1, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgArrowExp * arrowExpression2 = buildArrowExp (
      variableDeclarations->getReference (set), buildOpaqueVarRefExp (size,
          subroutineScope));

  SgAddOp * addExpression2 = buildAddOp (variableDeclarations->getReference (
      getIterationCounterVariableName (1)), buildIntVal (1));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (arrowExpression2,
      addExpression2);

  SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression2,
      variableDeclarations->getReference (numberOfThreads));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (sliceEnd), divideExpression2);

  appendStatement (assignmentStatement2, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  appendStatement (createThreadLoopStatements (), loopBody);

  /*
   * ======================================================
   * For loop statement
   * ======================================================
   */

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      variableDeclarations->getReference (numberOfThreads));

  SgPlusPlusOp * strideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgForStatement * forLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  /*
   * ======================================================
   * OpenMP for statement
   * ======================================================
   */

  SgOmpParallelStatement * openmpForStatement = new SgOmpParallelStatement (
      RoseHelper::getFileInfo (), forLoopStatement);

  forLoopStatement->set_parent (openmpForStatement);

  return openmpForStatement;
}

void
CPPOpenMPHostSubroutineDirectLoop::createStatements ()
{
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  appendStatementList (
      createInitialiseNumberOfThreadsStatements ()->getStatementList (),
      subroutineScope);

  appendStatementList (createOpDatTypeCastStatements ()->getStatementList (),
      subroutineScope);

  createReductionPrologueStatements ();

  appendStatement (createOpenMPLoopStatements (), subroutineScope);

  createReductionEpilogueStatements ();
}

void
CPPOpenMPHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createOpenMPLocalVariableDeclarations ();

  createOpDatTypeCastVariableDeclarations ();

  createReductionDeclarations ();
}

CPPOpenMPHostSubroutineDirectLoop::CPPOpenMPHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope, CPPUserSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPOpenMPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
