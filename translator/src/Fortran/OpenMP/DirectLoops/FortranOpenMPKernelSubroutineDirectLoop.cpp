#include <FortranOpenMPKernelSubroutineDirectLoop.h>
#include <FortranParallelLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <OpenMP.h>

SgStatement *
FortranOpenMPKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
            variableDeclarations->getReference (OpenMP::sliceIterator),
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
            variableDeclarations->getReference (OpenMP::sliceIterator),
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression2 = buildAddOp (multiplyExpression2,
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgSubtractOp * subtractExpression2 = buildSubtractOp (addExpression2,
            buildIntVal (1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                multiplyExpression1, subtractExpression2, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            RoseHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (
                OP2::VariableNames::getOpDatName (i)), buildExprListExp (
                arraySubscriptExpression));

        actualParameters->append_expression (parameterExpression);
      }
      else
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (
                OP2::VariableNames::getOpDatName (i)));
      }
    }
  }

  SgFunctionSymbol * userSubroutineSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine (
          userSubroutine->getSubroutineName (), subroutineScope);

  SgFunctionCallExp * userSubroutineCall = buildFunctionCallExp (
      userSubroutineSymbol, actualParameters);

  return buildExprStatement (userSubroutineCall);
}

void
FortranOpenMPKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  SgAssignOp * initializationExpression = buildAssignOp (
      variableDeclarations->getReference (OpenMP::sliceIterator),
      variableDeclarations->getReference (OpenMP::sliceStart));

  SgSubtractOp * upperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (OpenMP::sliceEnd), buildIntVal (1));

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (createUserSubroutineCallStatement (), loopBody);

  SgFortranDo * doStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  appendStatement (doStatement, subroutineScope);
}

void
FortranOpenMPKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace SageBuilder;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgArrayType * arrayType = isSgArrayType (parallelLoop->getOpDatType (i));

      SgArrayType * newArrayType =
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              arrayType->get_base_type (), buildIntVal (0),
              new SgAsteriskShapeExp (RoseHelper::getFileInfo ()));

      variableDeclarations->add (
          OP2::VariableNames::getOpDatName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              OP2::VariableNames::getOpDatName (i), newArrayType,
              subroutineScope, formalParameters));
    }
  }
}

void
FortranOpenMPKernelSubroutineDirectLoop::createStatements ()
{
  createExecutionLoopStatements ();
}

void
FortranOpenMPKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  variableDeclarations->add (OpenMP::sliceIterator,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::sliceIterator, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));
}

void
FortranOpenMPKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  createOpDatFormalParameterDeclarations ();

  variableDeclarations->add (
      OpenMP::sliceStart,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OpenMP::sliceStart, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters));

  variableDeclarations->add (
      OpenMP::sliceEnd,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OpenMP::sliceEnd, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters));
}

FortranOpenMPKernelSubroutineDirectLoop::FortranOpenMPKernelSubroutineDirectLoop (
    SgScopeStatement * moduleScope,
    Subroutine <SgProcedureHeaderStatement> * userSubroutine,
    FortranParallelLoop * parallelLoop) :
  FortranOpenMPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "OpenMP kernel subroutine creation for direct loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
