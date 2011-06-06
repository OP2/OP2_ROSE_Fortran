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
FortranOpenMPKernelSubroutineDirectLoop::createUserSubroutineCall ()
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

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {

      if (parallelLoop->isReductionRequired (i) == false)
      {
        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
            localVariableDeclarations[OpenMP::sliceIterator]), buildIntVal (
            parallelLoop->get_OP_DAT_Dimension (i)));

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
            localVariableDeclarations[OpenMP::sliceIterator]), buildIntVal (
            parallelLoop->get_OP_DAT_Dimension (i)));

        SgAddOp * addExpression2 = buildAddOp (multiplyExpression2,
            buildIntVal (parallelLoop->get_OP_DAT_Dimension (i)));

        SgSubtractOp * subtractExpression2 = buildSubtractOp (addExpression2,
            buildIntVal (1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (),
                multiplyExpression1, subtractExpression2, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression =
            buildPntrArrRefExp (
                buildVarRefExp (
                    formalParameterDeclarations[get_OP_DAT_FormalParameterName (
                        i)]), buildExprListExp (arraySubscriptExpression));

        actualParameters->append_expression (parameterExpression);
      }
      else
      {
        actualParameters->append_expression (buildVarRefExp (
            formalParameterDeclarations[get_OP_DAT_FormalParameterName (i)]));
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
      localVariableDeclarations[OpenMP::sliceIterator]), buildVarRefExp (
      formalParameterDeclarations[OpenMP::sliceStart]));

  SgSubtractOp * upperBoundExpression = buildSubtractOp (buildVarRefExp (
      formalParameterDeclarations[OpenMP::sliceEnd]), buildIntVal (1));

  SgIntVal * strideExpression = buildIntVal (1);

  SgBasicBlock * loopBody = buildBasicBlock ();

  loopBody->append_statement (createUserSubroutineCall ());

  SgFortranDo * doStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, strideExpression,
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
      localVariableDeclarations[OpenMP::threadID]), functionCall);

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
  localVariableDeclarations[OpenMP::sliceIterator]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::sliceIterator, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope);

  localVariableDeclarations[OpenMP::threadID]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::threadID, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope);
}

void
FortranOpenMPKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  using SageBuilder::buildIntVal;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      SgArrayType * arrayType = isSgArrayType (
          parallelLoop->get_OP_DAT_Type (i));

      SgArrayType * newArrayType =
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              arrayType->get_base_type (), buildIntVal (0),
              new SgAsteriskShapeExp (ROSEHelper::getFileInfo ()));

      formalParameterDeclarations[get_OP_DAT_FormalParameterName (i)]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              get_OP_DAT_FormalParameterName (i), newArrayType,
              subroutineScope, formalParameters);
    }
  }

  formalParameterDeclarations[OpenMP::sliceStart]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OpenMP::sliceStart, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters);

  formalParameterDeclarations[OpenMP::sliceEnd]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OpenMP::sliceEnd, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters);
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
