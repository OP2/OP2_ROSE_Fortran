#include <FortranOpenMPKernelSubroutineDirectLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <ROSEHelper.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranOpenMPKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildPntrArrRefExp;

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {

      if (parallelLoop->isReductionRequired (i) == false)
      {
        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (OpenMP::sliceIterator)), buildIntVal (
            parallelLoop->getOpDatDimension (i)));

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (OpenMP::sliceIterator)), buildIntVal (
            parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression2 = buildAddOp (multiplyExpression2,
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgSubtractOp * subtractExpression2 = buildSubtractOp (addExpression2,
            buildIntVal (1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (),
                multiplyExpression1, subtractExpression2, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                VariableNames::getOpDatName (i))), buildExprListExp (
                arraySubscriptExpression));

        actualParameters->append_expression (parameterExpression);
      }
      else
      {
        actualParameters->append_expression (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatName (i))));
      }
    }
  }

  SgFunctionSymbol * userSubroutineSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine (userSubroutineName,
          subroutineScope);

  SgFunctionCallExp * userSubroutineCall = buildFunctionCallExp (
      userSubroutineSymbol, actualParameters);

  return buildExprStatement (userSubroutineCall);
}

void
FortranOpenMPKernelSubroutineDirectLoop::createUserSubroutineDoLoop ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;

  SgAssignOp * initializationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (OpenMP::sliceIterator)), buildVarRefExp (
      variableDeclarations->get (OpenMP::sliceStart)));

  SgSubtractOp * upperBoundExpression = buildSubtractOp (buildVarRefExp (
      variableDeclarations->get (OpenMP::sliceEnd)), buildIntVal (1));

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (createUserSubroutineCallStatement (), loopBody);

  SgFortranDo * doStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  appendStatement (doStatement, subroutineScope);
}

void
FortranOpenMPKernelSubroutineDirectLoop::initialiseThreadID ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("omp_get_thread_num",
          subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      buildExprListExp ());

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OpenMP::threadID)), functionCall);

  appendStatement (assignmentStatement, subroutineScope);
}

void
FortranOpenMPKernelSubroutineDirectLoop::createStatements ()
{
  initialiseThreadID ();

  createUserSubroutineDoLoop ();
}

void
FortranOpenMPKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  variableDeclarations->add (OpenMP::sliceIterator,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::sliceIterator, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (OpenMP::threadID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::threadID, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));
}

void
FortranOpenMPKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  using SageBuilder::buildIntVal;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgArrayType * arrayType = isSgArrayType (parallelLoop->getOpDatType (i));

      SgArrayType * newArrayType =
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              arrayType->get_base_type (), buildIntVal (0),
              new SgAsteriskShapeExp (ROSEHelper::getFileInfo ()));

      variableDeclarations->add (
          VariableNames::getOpDatName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getOpDatName (i), newArrayType, subroutineScope,
              formalParameters));
    }
  }

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

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranOpenMPKernelSubroutineDirectLoop::FortranOpenMPKernelSubroutineDirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranOpenMPKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope)
{
  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
