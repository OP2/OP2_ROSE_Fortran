#include <FortranOpenMPHostSubroutine.h>
#include <FortranKernelSubroutine.h>
#include <FortranParallelLoop.h>
#include <FortranOpenMPModuleDeclarations.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <CompilerGeneratedNames.h>
#include <OpenMP.h>

void
FortranOpenMPHostSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Create the inner loop
   * ======================================================
   */

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i))
    {
      SgAssignOp * innerLoopInitializationExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgSubtractOp * innerLoopUpperBoundExpression = buildSubtractOp (
          buildIntVal (parallelLoop->getOpDatDimension (i)), buildIntVal (1));

      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), buildIntVal (64));

      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), multiplyExpression1);

      SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          addExpression1);

      SgAddOp * addExpression2 = buildAddOp (buildIntVal (
          parallelLoop->getOpDatDimension (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
          moduleDeclarations->getGlobalOpDatDeclaration (i), addExpression2);

      SgAddOp * addExpression3 = buildAddOp (buildIntVal (
          parallelLoop->getOpDatDimension (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgPntrArrRefExp * arrayIndexExpression3 = buildPntrArrRefExp (
          moduleDeclarations->getGlobalOpDatDeclaration (i), addExpression3);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayIndexExpression3, buildAddOp (arrayIndexExpression1,
              arrayIndexExpression2));

      innerLoopBody->append_statement (assignmentStatement);

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopInitializationExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      outerLoopBody->append_statement (innerLoopStatement);
    }
  }

  /*
   * ======================================================
   * Create the outer loop
   * ======================================================
   */

  SgAssignOp * outerLoopInitializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgSubtractOp * outerLoopUpperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (OpenMP::numberOfThreads),
      buildIntVal (1));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
}

void
FortranOpenMPHostSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Create the inner loop
   * ======================================================
   */

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i))
    {
      SgAssignOp * innerLoopInitializationExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgSubtractOp * innerLoopUpperBoundExpression = buildSubtractOp (
          buildIntVal (parallelLoop->getOpDatDimension (i)), buildIntVal (1));

      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), buildIntVal (64));

      SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
          getIterationCounterVariableName (2)), multiplyExpression);

      SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          addExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayIndexExpression, buildIntVal (0));

      innerLoopBody->append_statement (assignmentStatement);

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopInitializationExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      outerLoopBody->append_statement (innerLoopStatement);
    }
  }

  /*
   * ======================================================
   * Create the outer loop
   * ======================================================
   */

  SgAssignOp * outerLoopInitializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgSubtractOp * outerLoopUpperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (OpenMP::numberOfThreads),
      buildIntVal (1));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
}

void
FortranOpenMPHostSubroutine::createReductionDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction local variable declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (getIterationCounterVariableName (1),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (getIterationCounterVariableName (2),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i))
    {
      string const & variableName = getOpDatLocalName (i);

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildIntVal (64),
          buildIntVal (64));

      SgAddOp * addExpression = buildAddOp (buildIntVal (
          parallelLoop->getOpDatDimension (i)), multiplyExpression);

      SgSubtractOp * subtractExpression = buildSubtractOp (addExpression,
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgType * newArrayType =
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              parallelLoop->getOpDatBaseType (i), buildIntVal (0),
              subtractExpression);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, newArrayType, subroutineScope));
    }
  }
}

void
FortranOpenMPHostSubroutine::initialiseNumberOfThreadsStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise number of threads",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("omp_get_max_threads",
          subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      buildExprListExp ());

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::numberOfThreads),
      functionCall);

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::numberOfThreads),
      buildIntVal (1));

  appendStatement (assignmentStatement2, subroutineScope);

  addTextForUnparser (assignmentStatement1, "#ifdef _OPENMP\n",
      AstUnparseAttribute::e_after);

  addTextForUnparser (assignmentStatement2, "#else\n",
      AstUnparseAttribute::e_before);

  addTextForUnparser (assignmentStatement2, "#endif\n",
      AstUnparseAttribute::e_after);
}

FortranOpenMPHostSubroutine::FortranOpenMPHostSubroutine (
    SgScopeStatement * moduleScope, FortranKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranOpenMPModuleDeclarations * moduleDeclarations) :
  FortranHostSubroutine (moduleScope, kernelSubroutine, parallelLoop),
      moduleDeclarations (moduleDeclarations)
{
}
