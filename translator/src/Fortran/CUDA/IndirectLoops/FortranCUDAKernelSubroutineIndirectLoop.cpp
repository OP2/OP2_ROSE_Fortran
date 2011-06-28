#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranCUDAKernelSubroutineIndirectLoop.h>
#include <CommonNamespaces.h>
#include <ROSEHelper.h>
#include <Plan.h>

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
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprListExp;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", 2);

  SgExprListExp * userDeviceSubroutineParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    int dim = parallelLoop->getOpDatDimension (i);

    SgExpression * parameterExpression = buildIntVal (1);

    if (parallelLoop->getOpMapValue (i) == GLOBAL)
    {
      if (parallelLoop->getOpAccessValue (i) == READ_ACCESS)
      {
        Debug::getInstance ()->debugMessage ("OP_GBL with read access", 7);

        /*
         * ======================================================
         * Case of global variable accessed in read mode:
         * we directly access the device variable, by
         * passing the kernel the variable name in positions
         * 0:argSize%<devVarName>-1. The name of the proper field
         * is obtained by appending "argument", <i>, and "_Size"
         * ======================================================
         */

        string const variableName = VariableNames::getOpDatSizeName (i);

        SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
            variableDeclarations->get (CommonVariableNames::argsSizes)),
            buildOpaqueVarRefExp (variableName, subroutineScope));

        SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
            buildIntVal (1));

        SgSubscriptExpression * subscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (), buildIntVal (
                0), subtractExpression, buildIntVal (1));

        subscriptExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatName (i))),
            subscriptExpression);
      }
      else
      {
        Debug::getInstance ()->debugMessage ("OP_GBL with write access", 7);

        /*
         * ======================================================
         * Case of global variable accessed NOT in read mode:
         * we access the corresponding local thread variable
         * ======================================================
         */

        parameterExpression = buildVarRefExp (variableDeclarations->get (
            VariableNames::getOpDatLocalName (i)));
      }
    }
    else if (parallelLoop->getOpMapValue (i) == INDIRECT
        && parallelLoop->getOpAccessValue (i) == INC_ACCESS)
    {
      Debug::getInstance ()->debugMessage (
          "Indirect OP_DAT with increment access", 7);

      parameterExpression = buildVarRefExp (variableDeclarations->get (
          VariableNames::getOpDatLocalName (i)));
    }
    else if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      Debug::getInstance ()->debugMessage (
          "Indirect OP_DAT with read/write access", 7);

      SgAddOp
          * addExpression1 =
              buildAddOp (
                  buildVarRefExp (variableDeclarations->get (
                      CommonVariableNames::iterationCounter1)),
                  buildVarRefExp (
                      variableDeclarations->get (
                          IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockOffsetShared)));

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getGlobalToLocalMappingName (i))), addExpression1);

      std::cout << i << std::endl;
      SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::opDatDimensions)),
          buildVarRefExp (
              opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      std::cout << i << std::endl;

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
          dotExpression1);

      SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getNumberOfBytesVariableName (i))),
          multiplyExpression1);

      SgAddOp
          * addExpression3 =
              buildAddOp (
                  buildVarRefExp (variableDeclarations->get (
                      CommonVariableNames::iterationCounter1)),
                  buildVarRefExp (
                      variableDeclarations->get (
                          IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockOffsetShared)));

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getGlobalToLocalMappingName (i))), addExpression3);

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (arrayExpression2,
          dotExpression1);

      SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getNumberOfBytesVariableName (i))),
          multiplyExpression2);

      SgAddOp * addExpression5 = buildAddOp (addExpression4, dotExpression1);

      SgSubscriptExpression * subscriptExpression = new SgSubscriptExpression (
          ROSEHelper::getFileInfo (), addExpression2, addExpression5,
          buildIntVal (1));

      subscriptExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

      parameterExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::autoshared)),
          buildExprListExp (subscriptExpression));
    }
    else if (parallelLoop->getOpMapValue (i) == DIRECT)
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT", 7);

      if (parallelLoop->getNumberOfIndirectOpDats () > 0)
      {
        SgAddOp
            * addExpression1 =
                buildAddOp (
                    buildVarRefExp (variableDeclarations->get (
                        CommonVariableNames::iterationCounter1)),
                    buildVarRefExp (
                        variableDeclarations->get (
                            IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockOffsetShared)));

        SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
            variableDeclarations->get (CommonVariableNames::opDatDimensions)),
            buildVarRefExp (opDatDimensionsDeclaration->getOpDatDimensionField (
                i)));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (addExpression1,
            dotExpression);

        SgAddOp * addExpression2 = buildAddOp (multiplyExpression,
            dotExpression);

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (),
                multiplyExpression, addExpression2, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatName (i))),
            arraySubscriptExpression);
      }
      else if (dim == 1)
      {
        SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
            variableDeclarations->get (CommonVariableNames::opDatDimensions)),
            buildVarRefExp (opDatDimensionsDeclaration->getOpDatDimensionField (
                i)));

        SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
            buildIntVal (1));

        SgAddOp * addExpression = buildAddOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), subtractExpression);

        SgSubscriptExpression * subscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (),
                buildVarRefExp (variableDeclarations->get (
                    CommonVariableNames::iterationCounter1)), addExpression,
                buildIntVal (1));

        subscriptExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatName (i))),
            subscriptExpression);
      }
      else
      {
        parameterExpression = buildVarRefExp (variableDeclarations->get (
            VariableNames::getOpDatLocalName (i)));
      }
    }

    userDeviceSubroutineParameters->append_expression (parameterExpression);
  }

  return buildFunctionCallStmt (userSubroutineName, buildVoidType (),
      userDeviceSubroutineParameters, subroutineScope);
}

void
FortranCUDAKernelSubroutineIndirectLoop::createPointeredIncrementsOrWritesStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildWhileStmt;
  using SageInterface::appendStatement;

  unsigned int pindOffsOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      if (parallelLoop->getOpAccessValue (i) == WRITE_ACCESS
          || parallelLoop->getOpAccessValue (i) == RW_ACCESS
          || parallelLoop->getOpAccessValue (i) == INC_ACCESS)
      {
        SgDotExp * dotExpression1 = buildDotExp (buildOpaqueVarRefExp (
            CUDA::Fortran::threadidx, subroutineScope), buildOpaqueVarRefExp (
            CUDA::Fortran::x, subroutineScope));

        /*
         * ======================================================
         * Defining lower and upper bounds, and increment
         * ======================================================
         */

        SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (CommonVariableNames::opDatDimensions)),
            buildVarRefExp (opDatDimensionsDeclaration->getOpDatDimensionField (
                i)));

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (
                VariableNames::getIndirectionArgumentSizeName (i))),
            dotExpression2);

        SgLessThanOp * lessThanExpression1 = buildLessThanOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), multiplyExpression1);

        SgSubtractOp * subtractExpression1 = buildSubtractOp (dotExpression1,
            buildIntVal (1));

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

        SgExprStatement
            * assignmentStatement2 =
                buildAssignStatement (
                    buildVarRefExp (
                        variableDeclarations->get (
                            IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduled)),
                    functionCall);

        /*
         * ======================================================
         * Defining device variable accessing expressions
         * ======================================================
         */

        SgMultiplyOp
            * multiplyExpression2a =
                buildMultiplyOp (
                    buildVarRefExp (
                        variableDeclarations->get (
                            IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID)),
                    buildIntVal (
                        parallelLoop->getNumberOfDifferentIndirectOpDats ()));

        SgAddOp * addExpression2a = buildAddOp (buildIntVal (pindOffsOffset),
            multiplyExpression2a);

        SgPntrArrRefExp * arrayIndexExpression2a = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                PlanFunction::Fortran::pindOffs)), addExpression2a);

        SgDivideOp * divideExpression2 = buildDivideOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), dotExpression2);

        SgAddOp * addExpression2b = buildAddOp (arrayIndexExpression2a,
            divideExpression2);

        SgAddOp * addExpression2c = buildAddOp (buildIntVal (0),
            addExpression2b);

        SgPntrArrRefExp * arrayIndexExpression2b = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                VariableNames::getLocalToGlobalMappingName (i))),
            addExpression2c);

        SgMultiplyOp * multiplyExpression2b = buildMultiplyOp (
            arrayIndexExpression2b, dotExpression2);

        SgAddOp
            * addExpression2d =
                buildAddOp (
                    buildVarRefExp (
                        variableDeclarations->get (
                            IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduled)),
                    multiplyExpression2b);

        SgPntrArrRefExp * arrayIndexExpression2c = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                VariableNames::getOpDatName (i))), addExpression2d);

        /*
         * ======================================================
         * Defining autoshared variable accessing expressions
         * ======================================================
         */

        SgAddOp * addExpression2e = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                VariableNames::getNumberOfBytesVariableName (i))),
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)));

        SgPntrArrRefExp * arrayIndexExpression2d = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::autoshared)), addExpression2e);

        SgExprStatement * assignmentStatement3 = NULL;

        if (parallelLoop->getOpAccessValue (i) == INC_ACCESS)
        {
          /*
           * ======================================================
           * Increment Statement
           * ======================================================
           */

          SgAddOp * addExpression2f = buildAddOp (arrayIndexExpression2c,
              arrayIndexExpression2d);

          assignmentStatement3 = buildAssignStatement (arrayIndexExpression2c,
              addExpression2f);
        }

        if (parallelLoop->getOpAccessValue (i) == WRITE_ACCESS
            || parallelLoop->getOpAccessValue (i) == RW_ACCESS)
        {
          /*
           * ======================================================
           * Write or Read/Write Statement
           * ======================================================
           */

          assignmentStatement3 = buildAssignStatement (arrayIndexExpression2c,
              arrayIndexExpression2d);
        }

        /*
         * ======================================================
         * Defining loop body
         * ======================================================
         */

        SgDotExp * dotExpression3 = buildDotExp (buildOpaqueVarRefExp (
            CUDA::Fortran::blockdim, subroutineScope), buildOpaqueVarRefExp (
            CUDA::Fortran::x, subroutineScope));

        SgAddOp * addExpression2f = buildAddOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), dotExpression3);

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
FortranCUDAKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildWhileStmt;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  SgVarRefExp * iterationCounter_Reference1 = buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1));

  SgVarRefExp * variable_X_Reference1 = buildOpaqueVarRefExp (CUDA::Fortran::x,
      subroutineScope);

  SgVarRefExp * variable_Threadidx_Reference1 = buildOpaqueVarRefExp (
      CUDA::Fortran::threadidx, subroutineScope);

  SgDotExp * dotExpression1 = buildDotExp (variable_Threadidx_Reference1,
      variable_X_Reference1);

  SgSubtractOp * subtractExpression1 = buildSubtractOp (dotExpression1,
      buildIntVal (1));

  SgAssignOp * assignmentExpression1 = buildAssignOp (
      iterationCounter_Reference1, subtractExpression1);

  appendStatement (buildExprStatement (assignmentExpression1), subroutineScope);

  /*
   * ======================================================
   * While loop body
   * ======================================================
   */

  SgBasicBlock * loopBodyNestingLevel1 = buildBasicBlock ();

  SgAssignOp * loopBodyExpression1 = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::col2)), buildIntVal (-1));

  loopBodyNestingLevel1->append_statement (buildExprStatement (
      loopBodyExpression1));

  SgExpression * ifGuardNestingLevel1 = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::KernelSubroutine::VariableNames::nelem)));

  SgBasicBlock * ifBodyNestingLevel1 = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpAccessValue (i) == INC_ACCESS)
    {
      SgAssignOp * loopLowerBoundNestingLevel2 = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          buildIntVal (0));

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::opDatDimensions)),
          buildVarRefExp (
              opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      SgSubtractOp * subtractExpression2 = buildSubtractOp (dotExpression,
          buildIntVal (1));

      SgBasicBlock * loopBodyNestingLevel2 = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)));

      SgAssignOp * innerLoopExpression1 = buildAssignOp (arrayExpression1,
          buildIntVal (0));

      loopBodyNestingLevel2->append_statement (buildExprStatement (
          innerLoopExpression1));

      SgFortranDo * loopStatementNestingLevel2 =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopLowerBoundNestingLevel2, subtractExpression2,
              buildIntVal (1), loopBodyNestingLevel2);

      ifBodyNestingLevel1->append_statement (loopStatementNestingLevel2);
    }
  }

  SgStatement * userSubroutineCall = createUserSubroutineCallStatement ();

  ifBodyNestingLevel1->append_statement (userSubroutineCall);

  SgAddOp
      * addExpression2 =
          buildAddOp (
              buildVarRefExp (variableDeclarations->get (
                  CommonVariableNames::iterationCounter1)),
              buildVarRefExp (
                  variableDeclarations->get (
                      IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockOffsetShared)));

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pthrcol)),
      addExpression2);

  SgAssignOp * assignmentExpression2 = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::col2)), arrayExpression2);

  ifBodyNestingLevel1->append_statement (buildExprStatement (
      assignmentExpression2));

  SgIfStmt * ifStatementNestingLevel1 =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardNestingLevel1, ifBodyNestingLevel1);

  loopBodyNestingLevel1->append_statement (ifStatementNestingLevel1);

  /*
   * ======================================================
   * Increment adjustment statements
   * ======================================================
   */
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT
        && parallelLoop->getOpAccessValue (i) == INC_ACCESS)
    {
      SgVarRefExp * incrementAccessMap_Reference = buildVarRefExp (
          variableDeclarations->get (VariableNames::getIncrementAccessMapName (
              i)));

      SgVarRefExp * globalToLocalMappingArray_Reference = buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getGlobalToLocalMappingName (i)));

      SgAddOp
          * addExpression =
              buildAddOp (
                  buildVarRefExp (variableDeclarations->get (
                      CommonVariableNames::iterationCounter1)),
                  buildVarRefExp (
                      variableDeclarations->get (
                          IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockOffsetShared)));

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          globalToLocalMappingArray_Reference, addExpression);

      SgAssignOp * assignmentExpression = buildAssignOp (
          incrementAccessMap_Reference, arrayExpression);

      loopBodyNestingLevel1->append_statement (buildExprStatement (
          assignmentExpression));
    }
  }

  /*
   * ======================================================
   * Inner while loop
   * ======================================================
   */

  SgBasicBlock * loopBodyNestingLevel2 = buildBasicBlock ();

  SgBasicBlock * ifBodyNestingLevel2 = buildBasicBlock ();

  SgEqualityOp * ifGuardNestingLevel2 = buildEqualityOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::col2)), buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::col)));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT
        && parallelLoop->getOpAccessValue (i) == INC_ACCESS)
    {
      SgBasicBlock * loopBodyNestingLevel3 = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)));

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::opDatDimensions)),
          buildVarRefExp (
              opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getIncrementAccessMapName (
              i))), dotExpression2);

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          multiplyExpression);

      SgAddOp
          * addExpression2 = buildAddOp (buildVarRefExp (
              variableDeclarations->get (
                  VariableNames::getNumberOfBytesVariableName (i))),
              addExpression1);

      SgPntrArrRefExp * rhsExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::autoshared)),
          addExpression2);

      SgPntrArrRefExp * lhsExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::autoshared)),
          addExpression2);

      SgAssignOp * assignmentExpression = buildAssignOp (lhsExpression,
          buildAddOp (rhsExpression, arrayExpression1));

      loopBodyNestingLevel3->append_statement (buildExprStatement (
          assignmentExpression));

      SgAssignOp * loopLowerBoundNestingLevel3 = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          buildIntVal (0));

      SgSubtractOp * subtractExpression2 = buildSubtractOp (dotExpression2,
          buildIntVal (1));

      SgFortranDo * loopStatementNestingLevel3 =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopLowerBoundNestingLevel3, subtractExpression2,
              buildIntVal (1), loopBodyNestingLevel3);

      ifBodyNestingLevel2->append_statement (loopStatementNestingLevel3);
    }
  }

  SgIfStmt * ifStatementNestingLevel2 =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardNestingLevel2, ifBodyNestingLevel2);

  loopBodyNestingLevel2->append_statement (ifStatementNestingLevel2);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine ("syncthreads",
          subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp ();

  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  loopBodyNestingLevel2->append_statement (buildExprStatement (subroutineCall));

  SgAssignOp * loopLowerBoundNestingLevel2 = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::col)), buildIntVal (0));

  SgSubtractOp * loopUpperBoundNestingLevel2 = buildSubtractOp (buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::KernelSubroutine::VariableNames::ncolor)),
      buildIntVal (1));

  SgFortranDo * loopStatementNestingLevel2 =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          loopLowerBoundNestingLevel2, loopUpperBoundNestingLevel2,
          buildIntVal (1), loopBodyNestingLevel2);

  loopBodyNestingLevel1->append_statement (loopStatementNestingLevel2);

  /*
   * ======================================================
   * Increment of iteration variable in nesting level 1
   * ======================================================
   */

  SgVarRefExp * iterationVarNesting1Ref = buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1));

  SgStatement * incrementIterationVariableNesting1 = buildAssignStatement (
      iterationVarNesting1Ref, buildAddOp (iterationVarNesting1Ref,
          buildDotExp (buildOpaqueVarRefExp (CUDA::Fortran::blockdim,
              subroutineScope), buildOpaqueVarRefExp (CUDA::Fortran::x,
              subroutineScope))));

  loopBodyNestingLevel1->append_statement (incrementIterationVariableNesting1);

  /*
   * ======================================================
   * While loop guard
   * ======================================================
   */

  SgExpression * loopGuardNestingLevel1 = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::KernelSubroutine::VariableNames::nelems2)));

  SgWhileStmt * loopStatementNestingLevel1 = buildWhileStmt (
      loopGuardNestingLevel1, loopBodyNestingLevel1);

  loopStatementNestingLevel1->set_has_end_statement (true);

  appendStatement (loopStatementNestingLevel1, subroutineScope);
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

  unsigned int pindOffsOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      /*
       * ======================================================
       * Initialise the lower bound of the while loop
       * ======================================================
       */

      SgDotExp * dotExpression1 = buildDotExp (buildOpaqueVarRefExp (
          CUDA::Fortran::threadidx, subroutineScope), buildOpaqueVarRefExp (
          CUDA::Fortran::x, subroutineScope));

      SgSubtractOp * subtractExpression1 = buildSubtractOp (dotExpression1,
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

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getIndirectionArgumentSizeName (i))), buildIntVal (
          parallelLoop->getOpDatDimension (i)));

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

        SgExprListExp * modActualParameters = buildExprListExp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgFunctionCallExp * modFunctionCall = buildFunctionCallExp (
            modFunctionSymbol, modActualParameters);

        SgExprStatement
            * assignmentStatement3 =
                buildAssignStatement (
                    buildVarRefExp (
                        variableDeclarations->get (
                            IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduled)),
                    modFunctionCall);

        appendStatement (assignmentStatement3, loopBody);

        /*
         * ======================================================
         * Statement to index autoshared array
         * ======================================================
         */
        SgAddOp * addExpression4a = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                VariableNames::getNumberOfBytesVariableName (i))),
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)));

        SgPntrArrRefExp * arrayIndexExpression4a = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::autoshared)), addExpression4a);

        SgMultiplyOp
            * multiplyExpression4a =
                buildMultiplyOp (
                    buildVarRefExp (
                        variableDeclarations->get (
                            IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID)),
                    buildIntVal (
                        parallelLoop->getNumberOfDifferentIndirectOpDats ()));

        SgAddOp * addExpression4b = buildAddOp (buildIntVal (pindOffsOffset),
            multiplyExpression4a);

        SgPntrArrRefExp * arrayIndexExpression4b = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                PlanFunction::Fortran::pindOffs)), addExpression4b);

        SgDivideOp * divideExpression4 = buildDivideOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression4c = buildAddOp (arrayIndexExpression4b,
            divideExpression4);

        SgAddOp * addExpression4d = buildAddOp (buildIntVal (0),
            addExpression4c);

        SgPntrArrRefExp * arrayIndexExpression4c = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                VariableNames::getLocalToGlobalMappingName (i))),
            addExpression4d);

        SgMultiplyOp * multiplyExpression4b = buildMultiplyOp (
            arrayIndexExpression4c, buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgAddOp
            * addExpression4e =
                buildAddOp (
                    buildVarRefExp (
                        variableDeclarations->get (
                            IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduled)),
                    multiplyExpression4b);

        SgPntrArrRefExp * arrayIndexExpression4d = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                VariableNames::getOpDatName (i))), addExpression4e);

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
                VariableNames::getNumberOfBytesVariableName (i))),
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)));

        SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::autoshared)), addExpression4);

        SgExprStatement * assignmentStatement4 = buildAssignStatement (
            arrayIndexExpression4, buildIntVal (0));

        appendStatement (assignmentStatement4, loopBody);
      }

      /*
       * ======================================================
       * Statement to increment loop counter
       * ======================================================
       */
      SgDotExp * dotExpression5 = buildDotExp (buildOpaqueVarRefExp (
          CUDA::Fortran::blockdim, subroutineScope), buildOpaqueVarRefExp (
          CUDA::Fortran::x, subroutineScope));

      SgAddOp * addExpression5 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          dotExpression5);

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

  /*
   * ======================================================
   * All threads must synchronize before kernel execution
   * can proceed. Add the statement
   * ======================================================
   */

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine ("syncthreads",
          subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp ();

  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  appendStatement (buildExprStatement (subroutineCall), subroutineScope);
}

void
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseLocalVariablesStatements ()
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * Initialise round-up variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgVarRefExp * opDatDimensionsReference = buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::opDatDimensions));

      SgVarRefExp * fieldReference = buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i));

      SgDotExp * dotExpression = buildDotExp (opDatDimensionsReference,
          fieldReference);

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getIndirectionArgumentSizeName (i))),
          dotExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getRoundUpVariableName (i))), multiplyExpression);

      appendStatement (assignmentStatement, subroutineScope);
    }
  }

  /*
   * ======================================================
   * Initialise number of bytes variables
   * ======================================================
   */

  bool firstInitialization = true;
  unsigned int previous_OP_DAT_Location;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      if (firstInitialization)
      {
        firstInitialization = false;

        SgVarRefExp * nbytes_Reference = buildVarRefExp (
            variableDeclarations->get (
                VariableNames::getNumberOfBytesVariableName (i)));

        SgExprStatement * assignmentStatement = buildAssignStatement (
            nbytes_Reference, buildIntVal (0));

        appendStatement (assignmentStatement, subroutineScope);
      }
      else
      {
        SgVarRefExp * nbytes_Reference = buildVarRefExp (
            variableDeclarations->get (
                VariableNames::getNumberOfBytesVariableName (i)));

        SgVarRefExp * previous_nbytes_Reference = buildVarRefExp (
            variableDeclarations->get (
                VariableNames::getNumberOfBytesVariableName (
                    previous_OP_DAT_Location)));

        SgVarRefExp * previous_inRoundUp_Reference = buildVarRefExp (
            variableDeclarations->get (VariableNames::getRoundUpVariableName (
                previous_OP_DAT_Location)));

        SgAddOp * addExpression = buildAddOp (previous_nbytes_Reference,
            previous_inRoundUp_Reference);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            nbytes_Reference, addExpression);

        appendStatement (assignmentStatement, subroutineScope);
      }

      previous_OP_DAT_Location = i;
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

  /*
   * ======================================================
   * Build the if-guard
   * ======================================================
   */

  SgVarRefExp * threadidx_Reference = buildOpaqueVarRefExp (
      CUDA::Fortran::threadidx, subroutineScope);

  SgVarRefExp * x_Reference = buildOpaqueVarRefExp (CUDA::Fortran::x,
      subroutineScope);

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildSubtractOp (
      buildDotExp (threadidx_Reference, x_Reference), buildIntVal (1)),
      buildIntVal (0));

  /*
   * ======================================================
   * 1st statement
   * ======================================================
   */

  SgVarRefExp * blockID_Reference1 = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID));

  SgVarRefExp * blockOffset_Reference1 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::blockOffset));

  SgVarRefExp * pblkMap_Reference1 = buildVarRefExp (variableDeclarations->get (
      PlanFunction::Fortran::pblkMap));

  SgVarRefExp * blockidx_Reference1 = buildOpaqueVarRefExp (
      CUDA::Fortran::blockidx, subroutineScope);

  SgVarRefExp * x_Reference1 = buildOpaqueVarRefExp (CUDA::Fortran::x,
      subroutineScope);

  SgDotExp * blockIdxDotX = buildDotExp (blockidx_Reference1, x_Reference1);

  SgSubtractOp * subtractExpression1 = buildSubtractOp (blockIdxDotX,
      buildIntVal (1));

  SgAddOp * arrayIndexExpression1 = buildAddOp (subtractExpression1,
      blockOffset_Reference1);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (pblkMap_Reference1,
      arrayIndexExpression1);

  SgStatement * statement1 = buildAssignStatement (blockID_Reference1,
      arrayExpression1);

  ifBlock->append_statement (statement1);

  /*
   * ======================================================
   * 2nd statement
   * ======================================================
   */

  SgVarRefExp * pnelems_Reference2 = buildOpaqueVarRefExp (
      PlanFunction::Fortran::pnelems, subroutineScope);

  SgVarRefExp * nelem_Reference2 = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::nelem,
      subroutineScope);

  SgVarRefExp * blockID_Reference2 = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID));

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (pnelems_Reference2,
      blockID_Reference2);

  SgStatement * statement2 = buildAssignStatement (nelem_Reference2,
      arrayExpression2);

  ifBlock->append_statement (statement2);

  /*
   * ======================================================
   * 3rd statement
   * ======================================================
   */

  SgVarRefExp * poffset_Reference3 = buildOpaqueVarRefExp (
      PlanFunction::Fortran::poffset, subroutineScope);

  SgVarRefExp
      * offset_b_Reference3 =
          buildOpaqueVarRefExp (
              IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockOffsetShared,
              subroutineScope);

  SgVarRefExp * blockID_Reference3 = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID));

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (poffset_Reference3,
      blockID_Reference3);

  SgStatement * statement3 = buildAssignStatement (offset_b_Reference3,
      arrayExpression3);

  ifBlock->append_statement (statement3);

  /*
   * ======================================================
   * 4th statement: assignment of nelems2
   * ======================================================
   */

  SgVarRefExp * pNelems2_Reference3 = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::nelems2,
      subroutineScope);

  SgVarRefExp * blockdim_Reference1 = buildOpaqueVarRefExp (
      CUDA::Fortran::blockdim, subroutineScope);

  SgDotExp * blockdimxDotX = buildDotExp (blockdim_Reference1, x_Reference1);

  SgExpression * nelem2InitSubExpr = buildAddOp (buildIntVal (1),
      buildDivideOp (buildSubtractOp (nelem_Reference2, buildIntVal (1)),
          blockdimxDotX));

  SgExpression * nelem2InitExpr = buildMultiplyOp (blockdimxDotX,
      nelem2InitSubExpr);

  SgStatement * initNelems2 = buildAssignStatement (pNelems2_Reference3,
      nelem2InitExpr);

  ifBlock->append_statement (initNelems2);

  /*
   * ======================================================
   * 5th statement: assignment of ncolor
   * ======================================================
   */

  SgVarRefExp * pNcolor_Reference3 = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::ncolor,
      subroutineScope);

  SgVarRefExp * pnthrcol_Reference = buildOpaqueVarRefExp (
      PlanFunction::Fortran::pnthrcol, subroutineScope);

  SgPntrArrRefExp * pncolorsOfBlockid = buildPntrArrRefExp (pnthrcol_Reference,
      blockID_Reference3);

  SgStatement * initNcolor = buildAssignStatement (pNcolor_Reference3,
      pncolorsOfBlockid);

  ifBlock->append_statement (initNcolor);

  /*
   * ======================================================
   * assignment of ind_arg_size variables
   * ======================================================
   */

  unsigned int pindSizesArrayOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgVarRefExp * ind_argSize_Reference = buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getIndirectionArgumentSizeName (i)));

      SgVarRefExp * pindSizes_Reference4 = buildVarRefExp (
          variableDeclarations->get (PlanFunction::Fortran::pindSizes));

      SgVarRefExp * blockID_Reference4 = buildVarRefExp (
          variableDeclarations->get (
              IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID));

      SgAddOp * arrayIndexExpression = buildAddOp (buildIntVal (
          pindSizesArrayOffset), buildMultiplyOp (blockID_Reference4,
          buildIntVal (
              parallelLoop->getNumberOfDistinctIndirectOpDatArguments ())));

      SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
          pindSizes_Reference4, arrayIndexExpression);

      SgStatement * statement4 = buildAssignStatement (ind_argSize_Reference,
          arrayExpression4);

      ifBlock->append_statement (statement4);

      ++pindSizesArrayOffset;
    }
  }

  /*
   * ======================================================
   * Add the if statement with a NULL else block
   * ======================================================
   */

  SgIfStmt * ifStatement =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  appendStatement (ifStatement, subroutineScope);

  /*
   * ======================================================
   * All threads must synchronize before kernel execution
   * can proceed. Add the statement
   * ======================================================
   */

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine ("syncthreads",
          subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp ();

  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  appendStatement (buildExprStatement (subroutineCall), subroutineScope);
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

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pindSizes);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pindOffs);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pblkMap);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::poffset);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pnelems);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pnthrcol);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pthrcol);

  for (vector <string>::const_iterator it =
      fourByteIntegerArrayVariables.begin (); it
      != fourByteIntegerArrayVariables.end (); ++it)
  {
    SgIntVal * lowerBoundExpression = buildIntVal (0);

    SgVarRefExp * argsSizesReference = buildVarRefExp (
        variableDeclarations->get (CommonVariableNames::argsSizes));

    SgVarRefExp * fieldReference = buildVarRefExp (
        dataSizesDeclaration->getFieldDeclarations ()->get (*it + "Size"));

    SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
        fieldReference);

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

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::blockOffset);

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

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters", 2);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp * argsSizesReference = buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::argsSizes));

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclaration->getFieldDeclarations ()->get (
              VariableNames::getOpDatSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
          fieldReference);

      SgSubtractOp * upperBoundExpression = buildSubtractOp (
          fieldSelectionExpression, buildIntVal (1));

      /*
       * ======================================================
       * The base type of an OP_DAT must always be an array
       * ======================================================
       */

      SgType * opdatType = parallelLoop->getOpDatType (i);

      SgArrayType * baseArrayType = isSgArrayType (opdatType);

      variableDeclarations->add (
          VariableNames::getOpDatName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getOpDatName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  baseArrayType->get_base_type (), lowerBoundExpression,
                  upperBoundExpression), subroutineScope, formalParameters, 1,
              DEVICE));

      SgIntVal * lowerBoundExpression2 = buildIntVal (0);

      SgVarRefExp * argsSizesReference2 = buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::argsSizes));

      SgVarRefExp * fieldReference2 = buildVarRefExp (
          dataSizesDeclaration->getFieldDeclarations ()->get (
              VariableNames::getLocalToGlobalMappingSizeName (i)));

      SgDotExp * fieldSelectionExpression2 = buildDotExp (argsSizesReference2,
          fieldReference2);

      SgSubtractOp * upperBoundExpression2 = buildSubtractOp (
          fieldSelectionExpression2, buildIntVal (1));

      variableDeclarations->add (
          VariableNames::getLocalToGlobalMappingName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getLocalToGlobalMappingName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  FortranTypesBuilder::getFourByteInteger (),
                  lowerBoundExpression2, upperBoundExpression2),
              subroutineScope, formalParameters, 1, DEVICE));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp * argsSizesReference = buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::argsSizes));

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclaration->getFieldDeclarations ()->get (
              VariableNames::getGlobalToLocalMappingSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
          fieldReference);

      SgSubtractOp * upperBoundExpression = buildSubtractOp (
          fieldSelectionExpression, buildIntVal (1));

      variableDeclarations->add (
          VariableNames::getGlobalToLocalMappingName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getGlobalToLocalMappingName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  FortranTypesBuilder::getTwoByteInteger (),
                  lowerBoundExpression, upperBoundExpression), subroutineScope,
              formalParameters, 1, DEVICE));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == DIRECT)
    {
      string const variableName = VariableNames::getOpDatName (i);

      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp * argsSizesReference = buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::argsSizes));

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclaration->getFieldDeclarations ()->get (
              VariableNames::getOpDatSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
          fieldReference);

      SgSubtractOp * upperBoundExpression = buildSubtractOp (
          fieldSelectionExpression, buildIntVal (1));

      variableDeclarations->add (
          VariableNames::getOpDatName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getOpDatName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  parallelLoop->getOpDatType (i), lowerBoundExpression,
                  upperBoundExpression), subroutineScope, formalParameters, 1,
              DEVICE));
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

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = VariableNames::getNumberOfBytesVariableName (
        i);

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
          VariableNames::getNumberOfBytesVariableName (
              positionOfNbytes[parallelLoop->getOpDatVariableName (i)])));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = VariableNames::getRoundUpVariableName (i);

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
      string const variableName = VariableNames::getIncrementAccessMapName (i);

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
          VariableNames::getIndirectionArgumentSizeName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope, 1, SHARED));
    }
  }

  vector <string> sharedFourByteIntegerVariables;

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockOffsetShared);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::ncolor);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::nelem);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::nelems2);

  for (vector <string>::const_iterator it =
      sharedFourByteIntegerVariables.begin (); it
      != sharedFourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope, 1,
            SHARED));
  }

  SgExpression * upperBoundExpression = new SgAsteriskShapeExp (
      ROSEHelper::getFileInfo ());

  variableDeclarations->add (CommonVariableNames::autoshared,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::autoshared,
          FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getDoublePrecisionFloat (), 0,
              upperBoundExpression), subroutineScope, 1, SHARED));

  vector <string> fourByteIntegerVariables;

  fourByteIntegerVariables.push_back (CommonVariableNames::col);

  fourByteIntegerVariables.push_back (CommonVariableNames::col2);

  fourByteIntegerVariables.push_back (CommonVariableNames::iterationCounter1);

  fourByteIntegerVariables.push_back (CommonVariableNames::iterationCounter2);

  fourByteIntegerVariables.push_back (CommonVariableNames::upperBound);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduloResult);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduled);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::nbytes);

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
      CommonVariableNames::opDatDimensions,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          CommonVariableNames::opDatDimensions,
          opDatDimensionsDeclaration->getType (), subroutineScope,
          formalParameters, 1, DEVICE));

  variableDeclarations->add (
      CommonVariableNames::argsSizes,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          CommonVariableNames::argsSizes, dataSizesDeclaration->getType (),
          subroutineScope, formalParameters, 1, DEVICE));

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
}

FortranCUDAKernelSubroutineIndirectLoop::FortranCUDAKernelSubroutineIndirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  FortranCUDAKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope, opDatDimensionsDeclaration),
      dataSizesDeclaration (dataSizesDeclaration)
{
  Debug::getInstance ()->debugMessage ("<Kernel, Indirect, CUDA>", 5);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
