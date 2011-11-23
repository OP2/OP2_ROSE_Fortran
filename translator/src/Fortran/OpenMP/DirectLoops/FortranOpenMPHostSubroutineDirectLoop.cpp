#include "FortranOpenMPHostSubroutineDirectLoop.h"
#include "FortranOpenMPKernelSubroutine.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "CompilerGeneratedNames.h"
#include "OP2.h"
#include "RoseHelper.h"
#include "Debug.h"
#include "OpenMP.h"

SgStatement *
FortranOpenMPHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace ReductionVariableNames;

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (getOpDatLocalName (i)));
      }
      else
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (64));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                multiplyExpression, buildNullExpression (), buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            RoseHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (getReductionArrayHostName (i)),
            buildExprListExp (arraySubscriptExpression));

        actualParameters->append_expression (parameterExpression);
      }
    }
  }

  actualParameters->append_expression (variableDeclarations->getReference (
      OpenMP::sliceStart));

  actualParameters->append_expression (variableDeclarations->getReference (
      OpenMP::sliceEnd));

  SgFunctionSymbol * kernelSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine (
          calleeSubroutine->getSubroutineName (), subroutineScope);

  SgFunctionCallExp * kernelCall = buildFunctionCallExp (kernelSymbol,
      actualParameters);

  return buildExprStatement (kernelCall);
}

void
FortranOpenMPHostSubroutineDirectLoop::createOpenMPLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage ("Creating OpenMP for loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgAssignOp * initializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgSubtractOp * upperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (OpenMP::numberOfThreads),
      buildIntVal (1));

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgDotExp * dotExpression1 = buildDotExp (variableDeclarations->getReference (
      getOpSetCoreName ()), buildOpaqueVarRefExp (size, subroutineScope));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (dotExpression1,
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgDivideOp * divideExpression1 = buildDivideOp (multiplyExpression1,
      variableDeclarations->getReference (OpenMP::numberOfThreads));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::sliceStart),
      divideExpression1);

  SgDotExp * dotExpression2 = buildDotExp (variableDeclarations->getReference (
      getOpSetCoreName ()), buildOpaqueVarRefExp (size, subroutineScope));

  SgAddOp * addExpression2 = buildAddOp (variableDeclarations->getReference (
      getIterationCounterVariableName (1)), buildIntVal (1));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (dotExpression2,
      addExpression2);

  SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression2,
      variableDeclarations->getReference (OpenMP::numberOfThreads));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::sliceEnd), divideExpression2);

  appendStatement (assignmentStatement1, loopBody);

  appendStatement (assignmentStatement2, loopBody);

  appendStatement (createKernelFunctionCallStatement (), loopBody);

  SgFortranDo * doStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  appendStatement (doStatement, subroutineScope);

  std::vector <SgVarRefExp *> privateVariableReferences;

  privateVariableReferences.push_back (variableDeclarations->getReference (
      OpenMP::sliceStart));

  privateVariableReferences.push_back (variableDeclarations->getReference (
      OpenMP::sliceEnd));

  privateVariableReferences.push_back (variableDeclarations->getReference (
      getIterationCounterVariableName (1)));

  addTextForUnparser (doStatement, OpenMP::getParallelLoopDirectiveString ()
      + OpenMP::getPrivateClause (privateVariableReferences),
      AstUnparseAttribute::e_before);

  addTextForUnparser (doStatement,
      OpenMP::getEndParallelLoopDirectiveString (),
      AstUnparseAttribute::e_after);
}

void
FortranOpenMPHostSubroutineDirectLoop::createStatements ()
{
  using namespace SageInterface;

  appendStatement (createInitialiseNumberOfThreadsStatements (),
      subroutineScope);

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  createOpenMPLoopStatements ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
FortranOpenMPHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  createOpDatLocalVariableDeclarations ();

  createOpenMPLocalVariableDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}

FortranOpenMPHostSubroutineDirectLoop::FortranOpenMPHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope,
    FortranOpenMPKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop) :
  FortranOpenMPHostSubroutine (moduleScope, kernelSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "OpenMP host subroutine creation for direct loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
