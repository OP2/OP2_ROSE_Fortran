#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <FortranTypesBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranCUDAKernelSubroutineIndirectLoop.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <Plan.h>
#include <CUDA.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranCUDAKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildMultiplyOp;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * parameterExpression;

    if (parallelLoop->isGlobal (i))
    {
      parameterExpression = buildOpGlobalActualParameterExpression (i);
    }
    else if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isIncremented (i))
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with increment access", Debug::OUTER_LOOP_LEVEL,
            __FILE__, __LINE__);

        parameterExpression = buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::getOpDatLocalName (i)));
      }
      else
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with read/write access", Debug::OUTER_LOOP_LEVEL,
            __FILE__, __LINE__);

        string const autosharedVariableName =
            OP2::VariableNames::getAutosharedDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        SgAddOp * addExpression1 = buildAddOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), buildVarRefExp (
                variableDeclarations->get (OP2::VariableNames::offset)));

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getGlobalToLocalMappingName (i))),
            addExpression1);

        SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getDimensionsVariableDeclarationName (
                    userSubroutineName))), buildVarRefExp (
            opDatDimensionsDeclaration->getOpDatDimensionField (i)));

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
            dotExpression1);

        SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getNumberOfBytesVariableName (i))),
            multiplyExpression1);

        SgAddOp * addExpression3 = buildAddOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), buildVarRefExp (
                variableDeclarations->get (OP2::VariableNames::offset)));

        SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getGlobalToLocalMappingName (i))),
            addExpression3);

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (arrayExpression2,
            dotExpression1);

        SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getNumberOfBytesVariableName (i))),
            multiplyExpression2);

        SgAddOp * addExpression5 = buildAddOp (addExpression4, dotExpression1);

        SgSubscriptExpression * subscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                addExpression2, addExpression5, buildIntVal (1));

        subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclarations->get (autosharedVariableName)),
            buildExprListExp (subscriptExpression));
      }
    }
    else if (parallelLoop->isDirect (i))
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      parameterExpression = buildIntVal (1);

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::offset)));

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getDimensionsVariableDeclarationName (
                  userSubroutineName))), buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (addExpression1,
          dotExpression);

      SgAddOp * addExpression2 = buildAddOp (multiplyExpression, dotExpression);

      SgSubscriptExpression * arraySubscriptExpression =
          new SgSubscriptExpression (RoseHelper::getFileInfo (),
              multiplyExpression, addExpression2, buildIntVal (1));

      arraySubscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

      parameterExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
          arraySubscriptExpression);
    }

    ROSE_ASSERT (parameterExpression != NULL);

    actualParameters->append_expression (parameterExpression);
  }

  return buildFunctionCallStmt (userSubroutineName, buildVoidType (),
      actualParameters, subroutineScope);
}

void
FortranCUDAKernelSubroutineIndirectLoop::createPointeredIncrementsOrWritesStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildWhileStmt;
  using SageInterface::appendStatement;
  using std::string;

  unsigned int pindOffsOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (i)
          || parallelLoop->isIncremented (i))
      {
        string const autosharedVariableName =
            OP2::VariableNames::getAutosharedDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));
        /*
         * ======================================================
         * Defining lower and upper bounds, and increment
         * ======================================================
         */

        SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getDimensionsVariableDeclarationName (
                    userSubroutineName))), buildVarRefExp (
            opDatDimensionsDeclaration->getOpDatDimensionField (i)));

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getIndirectionArgumentSizeName (i))),
            dotExpression2);

        SgLessThanOp * lessThanExpression1 = buildLessThanOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), multiplyExpression1);

        SgSubtractOp * subtractExpression1 = buildSubtractOp (
            CUDA::getThreadId (THREAD_X, subroutineScope), buildIntVal (1));

        SgExprStatement * assignmentStatement1 = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), subtractExpression1);

        appendStatement (assignmentStatement1, subroutineScope);

        /*
         * ======================================================
         * Defining moduled variable assignment
         * ======================================================
         */

        SgFunctionSymbol * functionSymbol =
            FortranTypesBuilder::buildNewFortranFunction ("mod",
                subroutineScope);

        SgExprListExp * actualParameters = buildExprListExp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), dotExpression2);

        SgFunctionCallExp * functionCall = buildFunctionCallExp (
            functionSymbol, actualParameters);

        SgExprStatement * assignmentStatement2 = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::moduloResult)), functionCall);

        /*
         * ======================================================
         * Defining device variable accessing expressions
         * ======================================================
         */

        SgMultiplyOp * multiplyExpression2a = buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::blockID)),
            buildIntVal (parallelLoop->getNumberOfDifferentIndirectOpDats ()));

        SgAddOp * addExpression2a = buildAddOp (buildIntVal (pindOffsOffset),
            multiplyExpression2a);

        SgPntrArrRefExp * arrayIndexExpression2a = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::PlanFunction::pindOffs)), addExpression2a);

        SgDivideOp * divideExpression2 = buildDivideOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), dotExpression2);

        SgAddOp * addExpression2b = buildAddOp (arrayIndexExpression2a,
            divideExpression2);

        SgAddOp * addExpression2c = buildAddOp (buildIntVal (0),
            addExpression2b);

        SgPntrArrRefExp * arrayIndexExpression2b = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getLocalToGlobalMappingName (i))),
            addExpression2c);

        SgMultiplyOp * multiplyExpression2b = buildMultiplyOp (
            arrayIndexExpression2b, dotExpression2);

        SgAddOp * addExpression2d = buildAddOp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::moduloResult)),
            multiplyExpression2b);

        SgPntrArrRefExp * arrayIndexExpression2c = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatName (i))), addExpression2d);

        /*
         * ======================================================
         * Defining autoshared variable accessing expressions
         * ======================================================
         */

        SgAddOp * addExpression2e = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getNumberOfBytesVariableName (i))),
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)));

        SgPntrArrRefExp * arrayIndexExpression2d =
            buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
                autosharedVariableName)), addExpression2e);

        SgExprStatement * assignmentStatement3 = NULL;

        if (parallelLoop->isIncremented (i))
        {
          SgAddOp * addExpression2f = buildAddOp (arrayIndexExpression2c,
              arrayIndexExpression2d);

          assignmentStatement3 = buildAssignStatement (arrayIndexExpression2c,
              addExpression2f);
        }

        if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (i))
        {
          assignmentStatement3 = buildAssignStatement (arrayIndexExpression2c,
              arrayIndexExpression2d);
        }

        /*
         * ======================================================
         * Defining loop body
         * ======================================================
         */

        SgAddOp * addExpression2f = buildAddOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)),
            CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

        SgExprStatement * assignmentStatement4 = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), addExpression2f);

        SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement2,
            assignmentStatement3, assignmentStatement4);

        SgWhileStmt * loopStatement = buildWhileStmt (lessThanExpression1,
            loopBody);

        loopStatement->set_has_end_statement (true);

        appendStatement (loopStatement, subroutineScope);

      }
      pindOffsOffset++;
    }
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createInnerExecutionLoopStatements (
    SgScopeStatement * scope)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;
  using std::string;

  SgBasicBlock * ifBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      string const autosharedVariableName =
          OP2::VariableNames::getAutosharedDeclarationName (
              parallelLoop->getOpDatBaseType (i), parallelLoop->getSizeOfOpDat (
                  i));

      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)));

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getDimensionsVariableDeclarationName (
                  userSubroutineName))), buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getIncrementAccessMapName (i))),
          dotExpression);

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          multiplyExpression);

      SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getNumberOfBytesVariableName (i))),
          addExpression1);

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (autosharedVariableName)), addExpression2);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (autosharedVariableName)), addExpression2);

      SgAddOp * addExpression3 =
          buildAddOp (arrayExpression2, arrayExpression1);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression3, addExpression3);

      appendStatement (assignmentStatement, innerLoopBody);

      SgAssignOp * innerLoopLowerBoundExpression = buildAssignOp (
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)), buildIntVal (0));

      SgSubtractOp * innerLoopUpperBoundExpression = buildSubtractOp (
          dotExpression, buildIntVal (1));

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopLowerBoundExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      appendStatement (innerLoopStatement, ifBody);
    }
  }

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::col2)), buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::col)));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (ifStatement, loopBody);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      loopBody);

  SgAssignOp * lowerBoundExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::col)), buildIntVal (0));

  SgSubtractOp * upperBoundExpression = buildSubtractOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::ncolor)), buildIntVal (1));

  SgFortranDo
      * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              lowerBoundExpression, upperBoundExpression, buildIntVal (1),
              loopBody);

  appendStatement (loopStatement, scope);
}

void
FortranCUDAKernelSubroutineIndirectLoop::createIncrementAdjustmentStatements (
    SgScopeStatement * scope)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAddOp;
  using SageInterface::appendStatement;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      SgAddOp * addExpression = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::offset)));

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getGlobalToLocalMappingName (i))),
          addExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getIncrementAccessMapName (i))),
          arrayExpression);

      appendStatement (assignmentStatement, scope);
    }
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseLocalOpDatStatements (
    SgScopeStatement * scope)
{
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageInterface::appendStatement;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIncremented (i))
    {
      SgBasicBlock * loopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression, buildIntVal (0));

      appendStatement (assignmentStatement, loopBody);

      SgAssignOp * lowerBoundExpression = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          buildIntVal (0));

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getDimensionsVariableDeclarationName (
                  userSubroutineName))), buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      SgFortranDo * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              lowerBoundExpression, upperBoundExpression, buildIntVal (1),
              loopBody);

      appendStatement (loopStatement, scope);
    }
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildWhileStmt;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * Initialise outer loop counter
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)), subtractExpression1);

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Create outer loop body
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock ();

  /*
   * ======================================================
   * Initialise inner loop counter
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::col2)),
      buildIntVal (-1));

  appendStatement (assignmentStatement2, loopBody);

  SgBasicBlock * ifBody = buildBasicBlock ();

  createInitialiseLocalOpDatStatements (ifBody);

  appendStatement (createUserSubroutineCallStatement (), ifBody);

  SgAddOp * addExpression3 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::offset)));

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pthrcol)),
      addExpression3);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::col2)),
      arrayExpression3);

  appendStatement (assignmentStatement3, ifBody);

  SgExpression * ifGuardExpression = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::nelems)));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, loopBody);

  createIncrementAdjustmentStatements (loopBody);

  createInnerExecutionLoopStatements (loopBody);

  SgAddOp * addExpresssion4 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)), addExpresssion4);

  appendStatement (assignmentStatement4, loopBody);

  /*
   * ======================================================
   * While loop guard
   * ======================================================
   */

  SgLessThanOp * loopGuardExpression = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::nelems2)));

  SgWhileStmt * loopStatement = buildWhileStmt (loopGuardExpression, loopBody);

  loopStatement->set_has_end_statement (true);

  appendStatement (loopStatement, subroutineScope);
}

void
FortranCUDAKernelSubroutineIndirectLoop::createAutoSharedWhileLoopStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildWhileStmt;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageInterface::appendStatement;
  using std::string;

  unsigned int pindOffsOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const autosharedVariableName =
          OP2::VariableNames::getAutosharedDeclarationName (
              parallelLoop->getOpDatBaseType (i), parallelLoop->getSizeOfOpDat (
                  i));

      /*
       * ======================================================
       * Initialise the lower bound of the while loop
       * ======================================================
       */

      SgSubtractOp * subtractExpression1 = buildSubtractOp (
          CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
          buildIntVal (1));

      SgExprStatement * assignmentStatement1 = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter1)), subtractExpression1);

      appendStatement (assignmentStatement1, subroutineScope);

      /*
       * ======================================================
       * Initialise the upper bound of the while loop
       * ======================================================
       */

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getDimensionsVariableDeclarationName (
                  userSubroutineName))), buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getIndirectionArgumentSizeName (i))),
          dotExpression2);

      SgExprStatement * assignmentStatement2 = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::upperBound)), multiplyExpression2);

      appendStatement (assignmentStatement2, subroutineScope);

      /*
       * ======================================================
       * While loop body
       * ======================================================
       */

      SgBasicBlock * loopBody = buildBasicBlock ();

      /*
       * ======================================================
       * Statement to calculate modulus
       * ======================================================
       */

      if (parallelLoop->getOpAccessValue (i) != INC_ACCESS)
      {
        SgFunctionSymbol * modFunctionSymbol =
            FortranTypesBuilder::buildNewFortranFunction ("mod",
                subroutineScope);

        SgDotExp * dotExpression3 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getDimensionsVariableDeclarationName (
                    userSubroutineName))), buildVarRefExp (
            opDatDimensionsDeclaration->getOpDatDimensionField (i)));

        SgExprListExp * modActualParameters = buildExprListExp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), dotExpression3);

        SgFunctionCallExp * modFunctionCall = buildFunctionCallExp (
            modFunctionSymbol, modActualParameters);

        SgExprStatement * assignmentStatement3 = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::moduloResult)), modFunctionCall);

        appendStatement (assignmentStatement3, loopBody);

        /*
         * ======================================================
         * Statement to index autoshared array
         * ======================================================
         */
        SgAddOp * addExpression4a = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getNumberOfBytesVariableName (i))),
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)));

        SgPntrArrRefExp * arrayIndexExpression4a =
            buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
                autosharedVariableName)), addExpression4a);

        SgMultiplyOp * multiplyExpression4a = buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::blockID)),
            buildIntVal (parallelLoop->getNumberOfDifferentIndirectOpDats ()));

        SgAddOp * addExpression4b = buildAddOp (buildIntVal (pindOffsOffset),
            multiplyExpression4a);

        SgPntrArrRefExp * arrayIndexExpression4b = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::PlanFunction::pindOffs)), addExpression4b);

        SgDotExp * dotExpression4 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getDimensionsVariableDeclarationName (
                    userSubroutineName))), buildVarRefExp (
            opDatDimensionsDeclaration->getOpDatDimensionField (i)));

        SgDivideOp * divideExpression4 = buildDivideOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), dotExpression4);

        SgAddOp * addExpression4c = buildAddOp (arrayIndexExpression4b,
            divideExpression4);

        SgAddOp * addExpression4d = buildAddOp (buildIntVal (0),
            addExpression4c);

        SgPntrArrRefExp * arrayIndexExpression4c = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getLocalToGlobalMappingName (i))),
            addExpression4d);

        SgMultiplyOp * multiplyExpression4b = buildMultiplyOp (
            arrayIndexExpression4c, dotExpression4);

        SgAddOp * addExpression4e = buildAddOp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::moduloResult)),
            multiplyExpression4b);

        SgPntrArrRefExp * arrayIndexExpression4d = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatName (i))), addExpression4e);

        SgExprStatement * assignmentStatement4 = buildAssignStatement (
            arrayIndexExpression4a, arrayIndexExpression4d);

        appendStatement (assignmentStatement4, loopBody);

        pindOffsOffset++;
      }
      else
      {
        /*
         * ======================================================
         * Statement to index autoshared array
         * ======================================================
         */
        SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getNumberOfBytesVariableName (i))),
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)));

        SgPntrArrRefExp * arrayIndexExpression4 =
            buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
                autosharedVariableName)), addExpression4);

        SgExprStatement * assignmentStatement4 = buildAssignStatement (
            arrayIndexExpression4, buildIntVal (0));

        appendStatement (assignmentStatement4, loopBody);
      }

      /*
       * ======================================================
       * Statement to increment loop counter
       * ======================================================
       */
      SgAddOp * addExpression5 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

      SgExprStatement * assignmentStatement5 = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter1)), addExpression5);

      appendStatement (assignmentStatement5, loopBody);

      /*
       * ======================================================
       * While loop guard
       * ======================================================
       */
      SgExpression * loopGuard = buildLessThanOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::upperBound)));

      SgWhileStmt * whileStatement = buildWhileStmt (loopGuard, loopBody);

      whileStatement->set_has_end_statement (true);

      appendStatement (whileStatement, subroutineScope);
    }
  }

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);
}

void
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseLocalVariablesStatements ()
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Initialising local variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Initialise round-up variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Initialising round-up variable for '"
                + parallelLoop->getOpDatVariableName (i) + "'",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgVarRefExp * opDatDimensionsReference = buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getDimensionsVariableDeclarationName (
                    userSubroutineName)));

        SgVarRefExp * fieldReference = buildVarRefExp (
            opDatDimensionsDeclaration->getOpDatDimensionField (i));

        SgDotExp * dotExpression = buildDotExp (opDatDimensionsReference,
            fieldReference);

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getIndirectionArgumentSizeName (i))),
            dotExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getRoundUpVariableName (i))),
            multiplyExpression);

        appendStatement (assignmentStatement, subroutineScope);
      }
    }
  }

  /*
   * ======================================================
   * Initialise number of bytes variables
   * ======================================================
   */

  bool firstInitialization = true;

  for (unsigned int i = 1, lasti = 1; i
      <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      Debug::getInstance ()->debugMessage (
          "Initialising number of bytes variable for '"
              + parallelLoop->getOpDatVariableName (i) + "'",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->isIndirect (i))
      {
        if (firstInitialization)
        {
          firstInitialization = false;

          SgExprStatement * assignmentStatement = buildAssignStatement (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getNumberOfBytesVariableName (i))),
              buildIntVal (0));

          appendStatement (assignmentStatement, subroutineScope);
        }
        else
        {
          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::getNumberOfBytesVariableName (lasti))),
              buildIntVal (parallelLoop->getSizeOfOpDat (lasti)));

          SgDivideOp * divideExpression1 = buildDivideOp (multiplyExpression1,
              buildIntVal (parallelLoop->getSizeOfOpDat (i)));

          SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::getRoundUpVariableName (lasti))),
              buildIntVal (parallelLoop->getSizeOfOpDat (lasti)));

          SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression2,
              buildIntVal (parallelLoop->getSizeOfOpDat (i)));

          SgAddOp * addExpression = buildAddOp (divideExpression1,
              divideExpression2);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getNumberOfBytesVariableName (i))),
              addExpression);

          appendStatement (assignmentStatement, subroutineScope);
        }

        lasti = i;
      }
    }
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createThreadZeroStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildDivideOp;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * The body of the if statement
   * ======================================================
   */

  SgBasicBlock * ifBlock = buildBasicBlock ();

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildSubtractOp (
      CUDA::getThreadId (THREAD_X, subroutineScope), buildIntVal (1)),
      buildIntVal (0));

  /*
   * ======================================================
   * 1st statement
   * ======================================================
   */

  SgVarRefExp * pblkMap_Reference1 = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::pblkMap));

  SgSubtractOp * subtractExpression1 = buildSubtractOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgAddOp * arrayIndexExpression1 = buildAddOp (subtractExpression1,
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::blockOffset)));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (pblkMap_Reference1,
      arrayIndexExpression1);

  SgStatement * statement1 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::blockID)),
      arrayExpression1);

  ifBlock->append_statement (statement1);

  /*
   * ======================================================
   * 2nd statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (OP2::VariableNames::PlanFunction::pnelems,
          subroutineScope), buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::blockID)));

  SgStatement * statement2 = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::nelems, subroutineScope), arrayExpression2);

  ifBlock->append_statement (statement2);

  /*
   * ======================================================
   * 3rd statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (OP2::VariableNames::PlanFunction::poffset,
          subroutineScope), buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::blockID)));

  SgStatement * statement3 = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::offset, subroutineScope), arrayExpression3);

  ifBlock->append_statement (statement3);

  /*
   * ======================================================
   * 4th statement: assignment of nelems2
   * ======================================================
   */

  SgExpression * nelem2InitSubExpr = buildAddOp (buildIntVal (1),
      buildDivideOp (buildSubtractOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::nelems)), buildIntVal (
          1)), CUDA::getThreadBlockDimension (THREAD_X, subroutineScope)));

  SgExpression * nelem2InitExpr = buildMultiplyOp (
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
      nelem2InitSubExpr);

  SgStatement * initNelems2 = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::nelems2, subroutineScope), nelem2InitExpr);

  ifBlock->append_statement (initNelems2);

  /*
   * ======================================================
   * 5th statement: assignment of ncolor
   * ======================================================
   */

  SgPntrArrRefExp * pncolorsOfBlockid = buildPntrArrRefExp (
      buildOpaqueVarRefExp (OP2::VariableNames::PlanFunction::pnthrcol,
          subroutineScope), buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::blockID)));

  SgStatement * initNcolor = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::ncolor, subroutineScope), pncolorsOfBlockid);

  ifBlock->append_statement (initNcolor);

  /*
   * ======================================================
   * assignment of ind_arg_size variables
   * ======================================================
   */

  unsigned int pindSizesArrayOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgAddOp * arrayIndexExpression = buildAddOp (buildIntVal (
            pindSizesArrayOffset), buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::blockID)),
            buildIntVal (
                parallelLoop->getNumberOfDistinctIndirectOpDatArguments ())));

        SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::PlanFunction::pindSizes)),
            arrayIndexExpression);

        SgStatement * statement4 = buildAssignStatement (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getIndirectionArgumentSizeName (i))),
            arrayExpression4);

        ifBlock->append_statement (statement4);

        ++pindSizesArrayOffset;
      }
    }
  }

  /*
   * ======================================================
   * Add the if statement with a NULL else block
   * ======================================================
   */

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  appendStatement (ifStatement, subroutineScope);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);
}

void
FortranCUDAKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using std::string;
  using std::vector;

  vector <string> fourByteIntegerArrayVariables;

  fourByteIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pindSizes);

  fourByteIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pindOffs);

  fourByteIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pblkMap);

  fourByteIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::poffset);

  fourByteIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pnelems);

  fourByteIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pnthrcol);

  fourByteIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pthrcol);

  for (vector <string>::const_iterator it =
      fourByteIntegerArrayVariables.begin (); it
      != fourByteIntegerArrayVariables.end (); ++it)
  {
    SgIntVal * lowerBoundExpression = buildIntVal (0);

    string const sizeVariableName = *it + "Size";

    SgDotExp * fieldSelectionExpression = buildDotExp (buildVarRefExp (
        variableDeclarations->get (
            OP2::VariableNames::getDataSizesVariableDeclarationName (
                userSubroutineName))), buildVarRefExp (
        dataSizesDeclaration->getFieldDeclarations ()->get (sizeVariableName)));

    SgSubtractOp * upperBoundExpression = buildSubtractOp (
        fieldSelectionExpression, buildIntVal (1));

    variableDeclarations->add (
        *it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            *it, FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                FortranTypesBuilder::getFourByteInteger (),
                lowerBoundExpression, upperBoundExpression), subroutineScope,
            formalParameters, 1, DEVICE));
  }

  vector <string> fourByteIntegerVariables;

  fourByteIntegerVariables.push_back (
      OP2::VariableNames::PlanFunction::blockOffset);

  for (vector <string>::const_iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations->add (
        *it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            *it, FortranTypesBuilder::getFourByteInteger (), subroutineScope,
            formalParameters, 1, VALUE));
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgIntVal * lowerBoundExpression = buildIntVal (0);

        SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getDataSizesVariableDeclarationName (
                    userSubroutineName))), buildVarRefExp (
            dataSizesDeclaration->getFieldDeclarations ()->get (
                OP2::VariableNames::getOpDatSizeName (i))));

        SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
            buildIntVal (1));

        /*
         * ======================================================
         * The base type of an OP_DAT must always be an array
         * ======================================================
         */

        variableDeclarations->add (
            OP2::VariableNames::getOpDatName (i),
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                OP2::VariableNames::getOpDatName (i),
                FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                    parallelLoop->getOpDatBaseType (i), lowerBoundExpression,
                    upperBoundExpression), subroutineScope, formalParameters,
                1, DEVICE));

        SgIntVal * lowerBoundExpression2 = buildIntVal (0);

        SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getDataSizesVariableDeclarationName (
                    userSubroutineName))), buildVarRefExp (
            dataSizesDeclaration->getFieldDeclarations ()->get (
                OP2::VariableNames::getLocalToGlobalMappingSizeName (i))));

        SgSubtractOp * upperBoundExpression2 = buildSubtractOp (dotExpression2,
            buildIntVal (1));

        variableDeclarations->add (
            OP2::VariableNames::getLocalToGlobalMappingName (i),
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                OP2::VariableNames::getLocalToGlobalMappingName (i),
                FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                    FortranTypesBuilder::getFourByteInteger (),
                    lowerBoundExpression2, upperBoundExpression2),
                subroutineScope, formalParameters, 1, DEVICE));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getDataSizesVariableDeclarationName (
                  userSubroutineName))), buildVarRefExp (
          dataSizesDeclaration->getFieldDeclarations ()->get (
              OP2::VariableNames::getGlobalToLocalMappingSizeName (i))));

      SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      variableDeclarations->add (
          OP2::VariableNames::getGlobalToLocalMappingName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              OP2::VariableNames::getGlobalToLocalMappingName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  FortranTypesBuilder::getTwoByteInteger (),
                  lowerBoundExpression, upperBoundExpression), subroutineScope,
              formalParameters, 1, DEVICE));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const variableName = OP2::VariableNames::getOpDatName (i);

      if (parallelLoop->isDirect (i) || parallelLoop->isGlobalNonScalar (i))
      {
        SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getDataSizesVariableDeclarationName (
                    userSubroutineName))), buildVarRefExp (
            dataSizesDeclaration->getFieldDeclarations ()->get (
                OP2::VariableNames::getOpDatSizeName (i))));

        SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
            buildIntVal (1));

        variableDeclarations->add (
            variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName,
                FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                    parallelLoop->getOpDatType (i), buildIntVal (0),
                    upperBoundExpression), subroutineScope, formalParameters,
                1, DEVICE));
      }
      else if (parallelLoop->isGlobalScalar (i))
      {
        variableDeclarations->add (
            variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, parallelLoop->getOpDatType (i), subroutineScope,
                formalParameters, 1, VALUE));
      }
    }
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createStatements ()
{
  createThreadZeroStatements ();

  createInitialiseLocalVariablesStatements ();

  createAutoSharedWhileLoopStatements ();

  createExecutionLoopStatements ();

  createPointeredIncrementsOrWritesStatements ();
}

void
FortranCUDAKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using boost::lexical_cast;
  using std::string;
  using std::vector;

  createLocalThreadDeclarations ();

  createAutoSharedDeclarations ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName =
        OP2::VariableNames::getNumberOfBytesVariableName (i);

    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));

      positionOfNbytes[parallelLoop->getOpDatVariableName (i)] = i;
    }
    else
    {
      variableDeclarations->add (variableName, variableDeclarations->get (
          OP2::VariableNames::getNumberOfBytesVariableName (
              positionOfNbytes[parallelLoop->getOpDatVariableName (i)])));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = OP2::VariableNames::getRoundUpVariableName (
          i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpAccessValue (i) == INC_ACCESS)
    {
      string const variableName =
          OP2::VariableNames::getIncrementAccessMapName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          OP2::VariableNames::getIndirectionArgumentSizeName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope, 1, SHARED));
    }
  }

  vector <string> sharedFourByteIntegerVariables;

  sharedFourByteIntegerVariables.push_back (OP2::VariableNames::blockID);

  sharedFourByteIntegerVariables.push_back (OP2::VariableNames::offset);

  sharedFourByteIntegerVariables.push_back (OP2::VariableNames::ncolor);

  sharedFourByteIntegerVariables.push_back (OP2::VariableNames::nelems);

  sharedFourByteIntegerVariables.push_back (OP2::VariableNames::nelems2);

  for (vector <string>::const_iterator it =
      sharedFourByteIntegerVariables.begin (); it
      != sharedFourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope, 1,
            SHARED));
  }

  vector <string> fourByteIntegerVariables;

  fourByteIntegerVariables.push_back (OP2::VariableNames::col);

  fourByteIntegerVariables.push_back (OP2::VariableNames::col2);

  fourByteIntegerVariables.push_back (CommonVariableNames::iterationCounter1);

  fourByteIntegerVariables.push_back (CommonVariableNames::iterationCounter2);

  fourByteIntegerVariables.push_back (CommonVariableNames::upperBound);

  fourByteIntegerVariables.push_back (OP2::VariableNames::moduloResult);

  fourByteIntegerVariables.push_back (OP2::VariableNames::moduloResult);

  fourByteIntegerVariables.push_back (OP2::VariableNames::nbytes);

  for (vector <string>::const_iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations ->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  variableDeclarations->add (
      OP2::VariableNames::getDimensionsVariableDeclarationName (
          userSubroutineName),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::getDimensionsVariableDeclarationName (
              userSubroutineName), opDatDimensionsDeclaration->getType (),
          subroutineScope, formalParameters, 1, DEVICE));

  variableDeclarations->add (
      OP2::VariableNames::getDataSizesVariableDeclarationName (
          userSubroutineName),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::getDataSizesVariableDeclarationName (
              userSubroutineName), dataSizesDeclaration->getType (),
          subroutineScope, formalParameters, 1, DEVICE));

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
}

FortranCUDAKernelSubroutineIndirectLoop::FortranCUDAKernelSubroutineIndirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranReductionSubroutines * reductionSubroutines,
    FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  FortranCUDAKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope, reductionSubroutines,
      opDatDimensionsDeclaration), dataSizesDeclaration (dataSizesDeclaration)
{
  Debug::getInstance ()->debugMessage ("<Kernel, Indirect, CUDA>",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
