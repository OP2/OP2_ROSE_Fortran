#include "FortranOpenMPHostSubroutineIndirectLoop.h"
#include "FortranParallelLoop.h"
#include "FortranOpenMPKernelSubroutine.h"
#include "FortranOpenMPModuleDeclarationsIndirectLoop.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "OP2Definitions.h"
#include "OpenMP.h"
#include "Debug.h"

SgStatement *
FortranOpenMPHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating kernel function call statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  SgExprStatement * callStatement = buildFunctionCallStmt (
      calleeSubroutine->getSubroutineName (), buildVoidType (),
      actualParameters, subroutineScope);

  return callStatement;
}

void
FortranOpenMPHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating plan function execution statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Statement to initialise the block offset (before the
   * loops commence execution)
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (blockOffset), buildIntVal (0));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Build the inner loop
   * ======================================================
   */

  SgBasicBlock * innerLoopBody = buildBasicBlock ();

  /*
   * ======================================================
   * ====> Statements inside loop
   * ======================================================
   */

  appendStatement (createKernelFunctionCallStatement (), innerLoopBody);

  /*
   * ======================================================
   * ====> Lower and upper bounds
   * ======================================================
   */

  SgAssignOp * innerLoopInitializationExpression = buildAssignOp (
      variableDeclarations->getReference (blockID), buildIntVal (0));

  SgExpression * innerLoopUpperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (nblocks), buildIntVal (1));

  SgFortranDo * innerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          innerLoopInitializationExpression, innerLoopUpperBoundExpression,
          buildIntVal (1), innerLoopBody);

  /*
   * ======================================================
   * Build the outer loop
   * ======================================================
   */

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  /*
   * ======================================================
   * ====> Statements inside loop
   * ======================================================
   */

  SgAddOp * addExpression2 = buildAddOp (variableDeclarations->getReference (
      colour1), buildIntVal (1));

  SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (ncolblk), addExpression2);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (nblocks), arrayIndexExpression2);

  appendStatement (assignmentStatement2, outerLoopBody);

  appendStatement (innerLoopStatement, outerLoopBody);

  SgAddOp * addExpression3 = buildAddOp (variableDeclarations->getReference (
      blockOffset), variableDeclarations->getReference (nblocks));

  SgStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (blockOffset), addExpression3);

  appendStatement (assignmentStatement3, outerLoopBody);

  /*
   * ======================================================
   * ====> Lower and upper bounds
   * ======================================================
   */

  SgAssignOp * outerLoopInitializationExpression = buildAssignOp (
      variableDeclarations->getReference (colour1), buildIntVal (0));

  SgDotExp * dotExpression3 = buildDotExp (variableDeclarations->getReference (
      actualPlan), buildOpaqueVarRefExp (ncolors, subroutineScope));

  SgExpression * outerLoopUpperBoundExpression = buildSubtractOp (
      dotExpression3, buildIntVal (1));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
}

SgBasicBlock *
FortranOpenMPHostSubroutineIndirectLoop::createTransferOpDatStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (dimension, block));

      SgDotExp * dotExpression2 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (set, block));

      SgDotExp * dotExpression3 = buildDotExp (dotExpression2,
          buildOpaqueVarRefExp (size, block));

      SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
          dotExpression3);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          variableDeclarations->getReference (getOpDatCardinalityName (i)),
          multiplyExpression);

      appendStatement (assignmentStatement, block);

      SgDotExp * parameterExpression1 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (dataOnHost, block));

      SgVarRefExp * parameterExpression2 = variableDeclarations->getReference (
          getOpDatCoreName (i));

      SgStatement
          * callStatement =
              FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                  subroutineScope, parameterExpression1, parameterExpression2,
                  FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                      variableDeclarations->getReference (
                          getOpDatCardinalityName (i))));

      appendStatement (callStatement, block);
    }
  }

  return block;
}

void
FortranOpenMPHostSubroutineIndirectLoop::createExecutionPlanDeclarations ()
{
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;
  using std::vector;

  vector <string> fourByteIntegerArrays;

  fourByteIntegerArrays.push_back (args);
  fourByteIntegerArrays.push_back (idxs);
  fourByteIntegerArrays.push_back (maps);
  fourByteIntegerArrays.push_back (accesses);
  fourByteIntegerArrays.push_back (inds);

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

  vector <string> fourByteIntegerVariables;

  fourByteIntegerVariables.push_back (getIterationCounterVariableName (1));
  fourByteIntegerVariables.push_back (colour1);
  fourByteIntegerVariables.push_back (argsNumber);
  fourByteIntegerVariables.push_back (indsNumber);
  fourByteIntegerVariables.push_back (blockOffset);
  fourByteIntegerVariables.push_back (nblocks);

  for (vector <string>::iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }
}

void
FortranOpenMPHostSubroutineIndirectLoop::createOpenMPVariableDeclarations ()
{
  using namespace PlanFunctionVariableNames;

  variableDeclarations->add (OpenMP::numberOfThreads,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::numberOfThreads, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (blockID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          blockID, FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranOpenMPHostSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;

  appendStatement (createInitialiseNumberOfThreadsStatements (),
      subroutineScope);

  appendStatement (createPlanFunctionParametersPreparationStatements (),
      subroutineScope);

  SgFunctionCallExp * planFunctionCallExpression =
      createPlanFunctionCallExpression ();

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (planRet), planFunctionCallExpression);

  appendStatement (assignmentStatement2, subroutineScope);

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  appendStatement (createConvertPositionInPMapsStatements (), subroutineScope);

  appendStatement (createConvertPlanFunctionParametersStatements (),
      subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  createPlanFunctionExecutionStatements ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
FortranOpenMPHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createOpenMPVariableDeclarations ();

  createExecutionPlanDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}

FortranOpenMPHostSubroutineIndirectLoop::FortranOpenMPHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    FortranOpenMPKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations) :
  FortranOpenMPHostSubroutine (moduleScope, kernelSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "OpenMP host subroutine creation for indirect loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  variableDeclarations->addVisibilityToSymbolsFromOuterScope (
      moduleDeclarations->getDeclarations ());

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  //createStatements ();
}
