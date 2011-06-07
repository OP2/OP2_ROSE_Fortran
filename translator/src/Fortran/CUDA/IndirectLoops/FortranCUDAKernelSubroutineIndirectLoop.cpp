#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranCUDAKernelSubroutineIndirectLoop.h>
#include <CommonNamespaces.h>
#include <ROSEHelper.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranCUDAKernelSubroutineIndirectLoop::createUserSubroutineCall ()
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

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    int dim = parallelLoop->get_OP_DAT_Dimension (i);

    SgExpression * parameterExpression = buildIntVal (1);

    if (parallelLoop->get_OP_MAP_Value (i) == GLOBAL)
    {
      if (parallelLoop->get_OP_Access_Value (i) == READ_ACCESS)
      {

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

        SgExpression
            * argSizeField =
                buildDotExp (
                    buildVarRefExp (
                        variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]),
                    buildOpaqueVarRefExp (variableName, subroutineScope));

        SgExpression * minusOneExpression = buildSubtractOp (argSizeField,
            buildIntVal (1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (), buildIntVal (
                0), minusOneExpression, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());
        arraySubscriptExpression->setCompilerGenerated ();
        arraySubscriptExpression->setOutputInCodeGeneration ();

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclarations[VariableNames::getOpDatName (i)]),
            arraySubscriptExpression);
      }
      else
      {
        /*
         * ======================================================
         * Case of global variable accessed *not* in read mode:
         * we access the corresponding local thread variable
         * ======================================================
         */

        parameterExpression = buildVarRefExp (
            variableDeclarations[VariableNames::getOpDatLocalName (i)]);
      }
    }
    else if (parallelLoop->get_OP_MAP_Value (i) == INDIRECT
        && parallelLoop->get_OP_Access_Value (i) == INC_ACCESS)
    {
      parameterExpression = buildVarRefExp (
          variableDeclarations[VariableNames::getOpDatLocalName (i)]);
    }
    else if (parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp
          * autoshared_Reference =
              buildVarRefExp (
                  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]);

      SgVarRefExp * globalToLocalMappingArray_Reference = buildVarRefExp (
          variableDeclarations[VariableNames::getGlobalToLocalMappingName (i)]);

      SgVarRefExp * nbytes_Reference =
          buildVarRefExp (
              variableDeclarations[VariableNames::getNumberOfBytesVariableName (
                  i)]);

      SgAddOp
          * lowerBound_addExpression1 =
              buildAddOp (
                  buildVarRefExp (
                      variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]),
                  buildVarRefExp (
                      variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::offset_b]));

      SgPntrArrRefExp * lowerBound_arrayExpression = buildPntrArrRefExp (
          globalToLocalMappingArray_Reference, lowerBound_addExpression1);

      SgMultiplyOp * lowerBound_multiplyExpression = buildMultiplyOp (
          lowerBound_arrayExpression, buildIntVal (
              parallelLoop->get_OP_DAT_Dimension (i)));

      SgAddOp * lowerBound_addExpression2 = buildAddOp (nbytes_Reference,
          lowerBound_multiplyExpression);

      SgAddOp
          * upperBound_addExpression1 =
              buildAddOp (
                  buildVarRefExp (
                      variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]),
                  buildVarRefExp (
                      variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::offset_b]));

      SgPntrArrRefExp * upperBound_arrayExpression = buildPntrArrRefExp (
          globalToLocalMappingArray_Reference, upperBound_addExpression1);

      SgMultiplyOp * upperBound_multiplyExpression = buildMultiplyOp (
          upperBound_arrayExpression, buildIntVal (
              parallelLoop->get_OP_DAT_Dimension (i)));

      SgAddOp * upperBound_addExpression2 = buildAddOp (nbytes_Reference,
          upperBound_multiplyExpression);

      SgAddOp * upperBound_addExpression3 = buildAddOp (
          upperBound_addExpression2, buildIntVal (
              parallelLoop->get_OP_DAT_Dimension (i)));

      SgSubscriptExpression * subscriptExpression = new SgSubscriptExpression (
          ROSEHelper::getFileInfo (), lowerBound_addExpression2,
          upperBound_addExpression3, buildIntVal (1));
      subscriptExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

      parameterExpression = buildPntrArrRefExp (autoshared_Reference,
          buildExprListExp (subscriptExpression));

    }
    else if (parallelLoop->get_OP_MAP_Value (i) == DIRECT)
    {
      if (parallelLoop->getNumberOfIndirectDataSets () > 0)
      {
        SgExpression
            * deviceVarAccessDirectBegin =
                buildMultiplyOp (
                    buildAddOp (
                        buildVarRefExp (
                            variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]),
                        buildVarRefExp (
                            variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::offset_b])),
                    buildIntVal (dim));

        SgExpression * deviceVarAccessDirectEnd = buildAddOp (
            deviceVarAccessDirectBegin, buildIntVal (dim));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (),
                deviceVarAccessDirectBegin, deviceVarAccessDirectEnd,
                buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());
        arraySubscriptExpression->setCompilerGenerated ();
        arraySubscriptExpression->setOutputInCodeGeneration ();

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclarations[VariableNames::getOpDatName (i)]),
            arraySubscriptExpression);
      }
      else if (dim == 1)
      {
        SgExpression
            * nVarRef =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);
        SgExpression * nPlusDimMinusOneExpr = buildAddOp (nVarRef, buildIntVal (
            dim - 1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (), nVarRef,
                nPlusDimMinusOneExpr, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());
        arraySubscriptExpression->setCompilerGenerated ();
        arraySubscriptExpression->setOutputInCodeGeneration ();

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclarations[VariableNames::getOpDatName (i)]),
            arraySubscriptExpression);
      }
      else
      {
        parameterExpression = buildVarRefExp (
            variableDeclarations[VariableNames::getOpDatLocalName (i)]);
      }
    }

    userDeviceSubroutineParameters->append_expression (parameterExpression);
  }

  return buildFunctionCallStmt (userSubroutineName, buildVoidType (),
      userDeviceSubroutineParameters, subroutineScope);
}

void
FortranCUDAKernelSubroutineIndirectLoop::applyPointeredIncrementsOrWrites ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignOp;
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

  SgVarRefExp * variable_X_Reference1 = buildOpaqueVarRefExp (
      CUDA::Fortran::FieldNames::x, subroutineScope);

  SgVarRefExp * variable_Threadidx_Reference1 = buildOpaqueVarRefExp (
      CUDA::Fortran::VariableNames::threadidx, subroutineScope);

  SgDotExp * threadidxDotX = buildDotExp (variable_Threadidx_Reference1,
      variable_X_Reference1);

  unsigned int pindOffsOffset = 0;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      if (parallelLoop->get_OP_Access_Value (i) == WRITE_ACCESS
          || parallelLoop->get_OP_Access_Value (i) == RW_ACCESS
          || parallelLoop->get_OP_Access_Value (i) == INC_ACCESS)
      {

        /*
         * ======================================================
         * Defining lower and upper bounds, and increment
         * ======================================================
         */

        SgVarRefExp
            * iterationVarIncWrVarsRef =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

        SgVarRefExp * indArgSize_Reference2 = buildVarRefExp (
            localVariables_indArgSizes[i]);

        SgMultiplyOp * upperBoundUpdateIncWrVars = buildMultiplyOp (
            indArgSize_Reference2, buildIntVal (
                parallelLoop->get_OP_DAT_Dimension (i)));

        SgExpression * upperBoundUpdateIncWrVarsGuard = buildLessThanOp (
            iterationVarIncWrVarsRef, upperBoundUpdateIncWrVars);

        SgExpression * initialiseIterationVariable = buildAssignOp (
            iterationVarIncWrVarsRef, buildSubtractOp (threadidxDotX,
                buildIntVal (1)));

        appendStatement (buildExprStatement (initialiseIterationVariable),
            subroutineScope);

        /*
         * ======================================================
         * Defining moduled variable assignment
         * ======================================================
         */

        SgVarRefExp
            * moduled_Reference3 =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduled]);

        SgFunctionSymbol * modFunctionSymbol =
            FortranTypesBuilder::buildNewFortranFunction ("mod",
                subroutineScope);

        SgExprListExp * modActualParameters = buildExprListExp (
            iterationVarIncWrVarsRef, buildIntVal (
                parallelLoop->get_OP_DAT_Dimension (i)));

        SgFunctionCallExp * modFunctionCall = buildFunctionCallExp (
            modFunctionSymbol, modActualParameters);

        SgAssignOp * assignmentExpressionModuled = buildAssignOp (
            moduled_Reference3, modFunctionCall);

        /*
         * ======================================================
         * Defining device variable accessing expressions
         * ======================================================
         */

        SgVarRefExp
            * blockId_Reference4 =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID]);

        SgVarRefExp
            * pindOffs_Reference4 =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffs]);

        SgMultiplyOp * multiplyExpression4_a = buildMultiplyOp (
            blockId_Reference4, buildIntVal (
                parallelLoop->getNumberOfDifferentIndirectDataSets ()));

        SgAddOp * addExpression4_a = buildAddOp (buildIntVal (pindOffsOffset),
            multiplyExpression4_a);

        SgPntrArrRefExp * arrayIndexExpression4_a = buildPntrArrRefExp (
            pindOffs_Reference4, addExpression4_a);

        SgVarRefExp
            * iterationCounter_Reference4_a =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

        SgDivideOp * divideExpression4 = buildDivideOp (
            iterationCounter_Reference4_a, buildIntVal (
                parallelLoop->get_OP_DAT_Dimension (i)));

        SgAddOp * addExpression4_b = buildAddOp (arrayIndexExpression4_a,
            divideExpression4);

        SgAddOp * addExpression4_c = buildAddOp (buildIntVal (0),
            addExpression4_b);

        SgVarRefExp
            * pindMaps_Reference4 =
                buildVarRefExp (
                    variableDeclarations[VariableNames::getLocalToGlobalMappingName (
                        i)]);

        SgPntrArrRefExp * arrayIndexExpression4_b = buildPntrArrRefExp (
            pindMaps_Reference4, addExpression4_c);

        SgMultiplyOp * multiplyExpression4_b = buildMultiplyOp (
            arrayIndexExpression4_b, buildIntVal (
                parallelLoop->get_OP_DAT_Dimension (i)));

        SgVarRefExp
            * moduled_Reference =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduled]);

        SgAddOp * addExpression4_d = buildAddOp (moduled_Reference,
            multiplyExpression4_b);

        SgVarRefExp * OP_DAT_Reference4 = buildVarRefExp (
            variableDeclarations[VariableNames::getOpDatName (i)]);

        SgPntrArrRefExp * deviceVarField = buildPntrArrRefExp (
            OP_DAT_Reference4, addExpression4_d);

        /*
         * ======================================================
         * Defining autoshared variable accessing expressions
         * ======================================================
         */

        SgVarRefExp
            * autoshared_Reference4 =
                buildVarRefExp (
                    variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]);

        SgVarRefExp
            * iterationCounter_Reference4 =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

        SgVarRefExp
            * nbytes_Reference4 =
                buildVarRefExp (
                    variableDeclarations[VariableNames::getNumberOfBytesVariableName (
                        i)]);

        SgAddOp * addExpression4 = buildAddOp (nbytes_Reference4,
            iterationCounter_Reference4);

        SgPntrArrRefExp * autosharedVarField = buildPntrArrRefExp (
            autoshared_Reference4, addExpression4);

        SgAssignOp * actualUpdate = NULL;

        if (parallelLoop->get_OP_Access_Value (i) == INC_ACCESS)
        {
          /*
           * ======================================================
           * Increment Statement
           * ======================================================
           */

          actualUpdate = buildAssignOp (deviceVarField, buildAddOp (
              deviceVarField, autosharedVarField));
        }

        if (parallelLoop->get_OP_Access_Value (i) == WRITE_ACCESS
            || parallelLoop->get_OP_Access_Value (i) == RW_ACCESS)
        {
          /*
           * ======================================================
           * Write or ReadWrite Statement
           * ======================================================
           */

          actualUpdate = buildAssignOp (deviceVarField, autosharedVarField);
        }

        /*
         * ======================================================
         * Defining loop body and loop
         * ======================================================
         */

        SgExpression * incrementVarIncrWrVars = buildAssignOp (
            iterationVarIncWrVarsRef, buildAddOp (iterationVarIncWrVarsRef,
                buildDotExp (buildOpaqueVarRefExp (
                    CUDA::Fortran::VariableNames::blockdim, subroutineScope),
                    buildOpaqueVarRefExp (CUDA::Fortran::FieldNames::x,
                        subroutineScope))));

        SgBasicBlock * loopBodyUpdateIncWrVars = buildBasicBlock (
            buildExprStatement (assignmentExpressionModuled),
            buildExprStatement (actualUpdate), buildExprStatement (
                incrementVarIncrWrVars));

        SgWhileStmt * updateIncrWrVars = buildWhileStmt (
            upperBoundUpdateIncWrVarsGuard, loopBodyUpdateIncWrVars);

        updateIncrWrVars->set_has_end_statement (true);

        appendStatement (updateIncrWrVars, subroutineScope);

      }
      pindOffsOffset++;
    }
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createPlanWhileLoop ()
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

  SgVarRefExp
      * iterationCounter_Reference1 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

  SgVarRefExp * variable_X_Reference1 = buildOpaqueVarRefExp (
      CUDA::Fortran::FieldNames::x, subroutineScope);

  SgVarRefExp * variable_Threadidx_Reference1 = buildOpaqueVarRefExp (
      CUDA::Fortran::VariableNames::threadidx, subroutineScope);

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

  SgAssignOp
      * loopBodyExpression1 =
          buildAssignOp (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::col2]),
              buildIntVal (-1));

  loopBodyNestingLevel1->append_statement (buildExprStatement (
      loopBodyExpression1));

  SgExpression
      * ifGuardNestingLevel1 =
          buildLessThanOp (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]),
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::nelem]));

  SgBasicBlock * ifBodyNestingLevel1 = buildBasicBlock ();

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_Access_Value (i) == INC_ACCESS)
    {
      SgAssignOp
          * loopLowerBoundNestingLevel2 =
              buildAssignOp (
                  buildVarRefExp (
                      variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter2]),
                  buildIntVal (0));

      SgIntVal * loopUpperBoundNestingLevel2 = buildIntVal (
          parallelLoop->get_OP_DAT_Dimension (i) - 1);

      SgBasicBlock * loopBodyNestingLevel2 = buildBasicBlock ();

      SgPntrArrRefExp
          * arrayExpression1 =
              buildPntrArrRefExp (
                  buildVarRefExp (
                      variableDeclarations[VariableNames::getOpDatLocalName (i)]),
                  buildVarRefExp (
                      variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter2]));

      SgAssignOp * innerLoopExpression1 = buildAssignOp (arrayExpression1,
          buildIntVal (0));

      loopBodyNestingLevel2->append_statement (buildExprStatement (
          innerLoopExpression1));

      SgFortranDo * loopStatementNestingLevel2 =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopLowerBoundNestingLevel2, loopUpperBoundNestingLevel2,
              buildIntVal (1), loopBodyNestingLevel2);

      ifBodyNestingLevel1->append_statement (loopStatementNestingLevel2);
    }
  }

  SgStatement * userSubroutineCall = createUserSubroutineCall ();

  ifBodyNestingLevel1->append_statement (userSubroutineCall);

  SgAddOp
      * addExpression2 =
          buildAddOp (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]),
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::offset_b]));

  SgPntrArrRefExp
      * arrayExpression2 =
          buildPntrArrRefExp (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcol]),
              addExpression2);

  SgAssignOp
      * assignmentExpression2 =
          buildAssignOp (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::col2]),
              arrayExpression2);

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
  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_MAP_Value (i) == INDIRECT
        && parallelLoop->get_OP_Access_Value (i) == INC_ACCESS)
    {
      SgVarRefExp * incrementAccessMap_Reference = buildVarRefExp (
          localVariables_incrementAccessMaps[i]);

      SgVarRefExp * globalToLocalMappingArray_Reference = buildVarRefExp (
          variableDeclarations[VariableNames::getGlobalToLocalMappingName (i)]);

      SgAddOp
          * addExpression =
              buildAddOp (
                  buildVarRefExp (
                      variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]),
                  buildVarRefExp (
                      variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::offset_b]));

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

  SgEqualityOp
      * ifGuardNestingLevel2 =
          buildEqualityOp (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::col2]),
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::col]));

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_MAP_Value (i) == INDIRECT
        && parallelLoop->get_OP_Access_Value (i) == INC_ACCESS)
    {
      SgBasicBlock * loopBodyNestingLevel3 = buildBasicBlock ();

      SgVarRefExp
          * autoshared_Reference_LHS =
              buildVarRefExp (
                  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]);

      SgVarRefExp
          * autoshared_Reference_RHS =
              buildVarRefExp (
                  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]);

      SgVarRefExp * incrementAccessArrays_Reference = buildVarRefExp (
          variableDeclarations[VariableNames::getOpDatLocalName (i)]);

      SgVarRefExp * incrementAccessMap_Reference = buildVarRefExp (
          localVariables_incrementAccessMaps[i]);

      SgPntrArrRefExp
          * arrayExpression1 =
              buildPntrArrRefExp (
                  incrementAccessArrays_Reference,
                  buildVarRefExp (
                      variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter2]));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          incrementAccessMap_Reference, buildIntVal (
              parallelLoop->get_OP_DAT_Dimension (i)));

      SgAddOp
          * addExpression1 =
              buildAddOp (
                  buildVarRefExp (
                      variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter2]),
                  multiplyExpression);

      SgAddOp * addExpression2 = buildAddOp (
          buildVarRefExp (
              variableDeclarations[VariableNames::getNumberOfBytesVariableName (
                  i)]), addExpression1);

      SgPntrArrRefExp * rhsExpression = buildPntrArrRefExp (
          autoshared_Reference_RHS, addExpression2);

      SgPntrArrRefExp * lhsExpression = buildPntrArrRefExp (
          autoshared_Reference_LHS, addExpression2);

      SgAssignOp * assignmentExpression = buildAssignOp (lhsExpression,
          buildAddOp (rhsExpression, arrayExpression1));

      loopBodyNestingLevel3->append_statement (buildExprStatement (
          assignmentExpression));

      SgAssignOp
          * loopLowerBoundNestingLevel3 =
              buildAssignOp (
                  buildVarRefExp (
                      variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter2]),
                  buildIntVal (0));

      SgIntVal * loopUpperBoundNestingLevel3 = buildIntVal (
          parallelLoop->get_OP_DAT_Dimension (i) - 1);

      SgFortranDo * loopStatementNestingLevel3 =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopLowerBoundNestingLevel3, loopUpperBoundNestingLevel3,
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

  SgAssignOp
      * loopLowerBoundNestingLevel2 =
          buildAssignOp (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::col]),
              buildIntVal (0));

  SgSubtractOp
      * loopUpperBoundNestingLevel2 =
          buildSubtractOp (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::ncolor]),
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

  SgVarRefExp
      * iterationVarNesting1Ref =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

  SgStatement * incrementIterationVariableNesting1 = buildAssignStatement (
      iterationVarNesting1Ref, buildAddOp (iterationVarNesting1Ref,
          buildDotExp (buildOpaqueVarRefExp (
              CUDA::Fortran::VariableNames::blockdim, subroutineScope),
              buildOpaqueVarRefExp (CUDA::Fortran::FieldNames::x,
                  subroutineScope))));

  loopBodyNestingLevel1->append_statement (incrementIterationVariableNesting1);

  /*
   * ======================================================
   * While loop guard
   * ======================================================
   */

  SgExpression
      * loopGuardNestingLevel1 =
          buildLessThanOp (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]),
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::nelems2]));

  SgWhileStmt * loopStatementNestingLevel1 = buildWhileStmt (
      loopGuardNestingLevel1, loopBodyNestingLevel1);

  loopStatementNestingLevel1->set_has_end_statement (true);

  appendStatement (loopStatementNestingLevel1, subroutineScope);
}

void
FortranCUDAKernelSubroutineIndirectLoop::createAutosharedWhileLoops ()
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
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildWhileStmt;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageInterface::appendStatement;

  unsigned int pindOffsOffset = 0;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      /*
       * ======================================================
       * Initialise the lower bound of the while loop
       * ======================================================
       */

      SgVarRefExp
          * iterationCounter_Reference1 =
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

      SgVarRefExp * variable_X_Reference1 = buildOpaqueVarRefExp (
          CUDA::Fortran::FieldNames::x, subroutineScope);

      SgVarRefExp * variable_Threadidx_Reference1 = buildOpaqueVarRefExp (
          CUDA::Fortran::VariableNames::threadidx, subroutineScope);

      SgDotExp * dotExpression1 = buildDotExp (variable_Threadidx_Reference1,
          variable_X_Reference1);

      SgSubtractOp * subtractExpression1 = buildSubtractOp (dotExpression1,
          buildIntVal (1));

      SgAssignOp * assignmentExpression1 = buildAssignOp (
          iterationCounter_Reference1, subtractExpression1);

      appendStatement (buildExprStatement (assignmentExpression1),
          subroutineScope);

      /*
       * ======================================================
       * Initialise the upper bound of the while loop
       * ======================================================
       */

      SgVarRefExp
          * whileLoopBound_Reference2 =
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::whileLoopBound]);

      SgVarRefExp * indArgSize_Reference2 = buildVarRefExp (
          localVariables_indArgSizes[i]);

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
          indArgSize_Reference2, buildIntVal (
              parallelLoop->get_OP_DAT_Dimension (i)));

      SgAssignOp * assignmentExpression2 = buildAssignOp (
          whileLoopBound_Reference2, multiplyExpression2);

      appendStatement (buildExprStatement (assignmentExpression2),
          subroutineScope);

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

      if (parallelLoop->get_OP_Access_Value (i) != INC_ACCESS)
      {
        SgVarRefExp
            * iterationCounter_Reference3 =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

        SgVarRefExp
            * moduled_Reference3 =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduled]);

        SgFunctionSymbol * modFunctionSymbol =
            FortranTypesBuilder::buildNewFortranFunction ("mod",
                subroutineScope);

        SgExprListExp * modActualParameters = buildExprListExp (
            iterationCounter_Reference3, buildIntVal (
                parallelLoop->get_OP_DAT_Dimension (i)));

        SgFunctionCallExp * modFunctionCall = buildFunctionCallExp (
            modFunctionSymbol, modActualParameters);

        SgAssignOp * assignmentExpression3 = buildAssignOp (moduled_Reference3,
            modFunctionCall);

        loopBody->append_statement (buildExprStatement (assignmentExpression3));

        /*
         * ======================================================
         * Statement to index autoshared array
         * ======================================================
         */

        SgVarRefExp
            * autoshared_Reference4 =
                buildVarRefExp (
                    variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]);

        SgVarRefExp
            * iterationCounter_Reference4 =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

        SgVarRefExp
            * nbytes_Reference4 =
                buildVarRefExp (
                    variableDeclarations[VariableNames::getNumberOfBytesVariableName (
                        i)]);

        SgAddOp * addExpression4 = buildAddOp (nbytes_Reference4,
            iterationCounter_Reference4);

        SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (
            autoshared_Reference4, addExpression4);

        SgVarRefExp
            * blockId_Reference4 =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID]);

        SgVarRefExp
            * pindOffs_Reference4 =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffs]);

        SgMultiplyOp * multiplyExpression4_a = buildMultiplyOp (
            blockId_Reference4, buildIntVal (
                parallelLoop->getNumberOfDifferentIndirectDataSets ()));

        SgAddOp * addExpression4_a = buildAddOp (buildIntVal (pindOffsOffset),
            multiplyExpression4_a);

        SgPntrArrRefExp * arrayIndexExpression4_a = buildPntrArrRefExp (
            pindOffs_Reference4, addExpression4_a);

        SgVarRefExp
            * iterationCounter_Reference4_a =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

        SgDivideOp * divideExpression4 = buildDivideOp (
            iterationCounter_Reference4_a, buildIntVal (
                parallelLoop->get_OP_DAT_Dimension (i)));

        SgAddOp * addExpression4_b = buildAddOp (arrayIndexExpression4_a,
            divideExpression4);

        SgAddOp * addExpression4_c = buildAddOp (buildIntVal (0),
            addExpression4_b);

        SgVarRefExp
            * pindMaps_Reference4 =
                buildVarRefExp (
                    variableDeclarations[VariableNames::getLocalToGlobalMappingName (
                        i)]);

        SgPntrArrRefExp * arrayIndexExpression4_b = buildPntrArrRefExp (
            pindMaps_Reference4, addExpression4_c);

        SgMultiplyOp * multiplyExpression4_b = buildMultiplyOp (
            arrayIndexExpression4_b, buildIntVal (
                parallelLoop->get_OP_DAT_Dimension (i)));

        SgVarRefExp
            * moduled_Reference =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduled]);

        SgAddOp * addExpression4_d = buildAddOp (moduled_Reference,
            multiplyExpression4_b);

        SgVarRefExp * OP_DAT_Reference4 = buildVarRefExp (
            variableDeclarations[VariableNames::getOpDatName (i)]);

        SgPntrArrRefExp * arrayIndexExpression4_c = buildPntrArrRefExp (
            OP_DAT_Reference4, addExpression4_d);

        SgAssignOp * assignmentExpression4 = buildAssignOp (
            arrayIndexExpression4, arrayIndexExpression4_c);

        loopBody->append_statement (buildExprStatement (assignmentExpression4));

        pindOffsOffset++;
      }
      else
      {
        /*
         * ======================================================
         * Statement to index autoshared array
         * ======================================================
         */
        SgVarRefExp
            * autoshared_Reference4 =
                buildVarRefExp (
                    variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]);

        SgVarRefExp
            * iterationCounter_Reference4 =
                buildVarRefExp (
                    variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

        SgVarRefExp
            * nbytes_Reference4 =
                buildVarRefExp (
                    variableDeclarations[VariableNames::getNumberOfBytesVariableName (
                        i)]);

        SgAddOp * addExpression4 = buildAddOp (nbytes_Reference4,
            iterationCounter_Reference4);

        SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (
            autoshared_Reference4, addExpression4);

        SgAssignOp * assignmentExpression4 = buildAssignOp (
            arrayIndexExpression4, buildIntVal (0));

        loopBody->append_statement (buildExprStatement (assignmentExpression4));
      }

      /*
       * ======================================================
       * Statement to increment loop counter
       * ======================================================
       */

      SgVarRefExp
          * iterationCounter_Reference5_LHS =
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

      SgVarRefExp
          * iterationCounter_Reference5_RHS =
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

      SgVarRefExp * variable_X_Reference5 = buildOpaqueVarRefExp (
          CUDA::Fortran::FieldNames::x, subroutineScope);

      SgVarRefExp * variable_BlockDim_Reference5 = buildOpaqueVarRefExp (
          CUDA::Fortran::VariableNames::blockdim, subroutineScope);

      SgDotExp * dotExpression5 = buildDotExp (variable_BlockDim_Reference5,
          variable_X_Reference5);

      SgAddOp * addExpression5 = buildAddOp (iterationCounter_Reference5_RHS,
          dotExpression5);

      SgAssignOp * assignmentExpression5 = buildAssignOp (
          iterationCounter_Reference5_LHS, addExpression5);

      loopBody->append_statement (buildExprStatement (assignmentExpression5));

      /*
       * ======================================================
       * While loop guard
       * ======================================================
       */

      SgVarRefExp
          * iterationCounter_Reference6 =
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter]);

      SgVarRefExp
          * whileLoopBound_Reference6 =
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::whileLoopBound]);

      SgExpression * loopGuard = buildLessThanOp (iterationCounter_Reference6,
          whileLoopBound_Reference6);

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
FortranCUDAKernelSubroutineIndirectLoop::initialiseLocalVariables ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
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

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * inRoundUp_Reference = buildVarRefExp (
          variableDeclarations[VariableNames::getRoundUpVariableName (i)]);

      SgVarRefExp * indArgSize_Reference = buildVarRefExp (
          localVariables_indArgSizes[i]);

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          indArgSize_Reference, buildIntVal (
              parallelLoop->get_OP_DAT_Dimension (i)));

      SgAssignOp * assignmentExpression = buildAssignOp (inRoundUp_Reference,
          multiplyExpression);

      appendStatement (buildExprStatement (assignmentExpression),
          subroutineScope);
    }
  }

  /*
   * ======================================================
   * Initialise number of bytes variables
   * ======================================================
   */

  bool firstInitialization = true;
  unsigned int previous_OP_DAT_Location;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      if (firstInitialization)
      {
        firstInitialization = false;

        SgVarRefExp
            * nbytes_Reference =
                buildVarRefExp (
                    variableDeclarations[VariableNames::getNumberOfBytesVariableName (
                        i)]);

        SgAssignOp * assignmentExpression = buildAssignOp (nbytes_Reference,
            buildIntVal (0));

        appendStatement (buildExprStatement (assignmentExpression),
            subroutineScope);
      }
      else
      {
        SgVarRefExp
            * nbytes_Reference =
                buildVarRefExp (
                    variableDeclarations[VariableNames::getNumberOfBytesVariableName (
                        i)]);

        SgVarRefExp * previous_nbytes_Reference = buildVarRefExp (
            variableDeclarations[VariableNames::getNumberOfBytesVariableName (
                previous_OP_DAT_Location)]);

        SgVarRefExp * previous_inRoundUp_Reference = buildVarRefExp (
            variableDeclarations[VariableNames::getRoundUpVariableName (
                previous_OP_DAT_Location)]);

        SgAddOp * addExpression = buildAddOp (previous_nbytes_Reference,
            previous_inRoundUp_Reference);

        SgAssignOp * assignmentExpression = buildAssignOp (nbytes_Reference,
            addExpression);

        appendStatement (buildExprStatement (assignmentExpression),
            subroutineScope);
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
      CUDA::Fortran::VariableNames::threadidx, subroutineScope);

  SgVarRefExp * x_Reference = buildOpaqueVarRefExp (
      CUDA::Fortran::FieldNames::x, subroutineScope);

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildSubtractOp (
      buildDotExp (threadidx_Reference, x_Reference), buildIntVal (1)),
      buildIntVal (0));

  /*
   * ======================================================
   * 1st statement
   * ======================================================
   */

  SgVarRefExp
      * blockID_Reference1 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID]);

  SgVarRefExp
      * blockOffset_Reference1 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::blockOffset]);

  SgVarRefExp
      * pblkMap_Reference1 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMap]);

  SgVarRefExp * blockidx_Reference1 = buildOpaqueVarRefExp (
      CUDA::Fortran::VariableNames::blockidx, subroutineScope);

  SgVarRefExp * x_Reference1 = buildOpaqueVarRefExp (
      CUDA::Fortran::FieldNames::x, subroutineScope);

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
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnelems,
      subroutineScope);

  SgVarRefExp * nelem_Reference2 = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::nelem,
      subroutineScope);

  SgVarRefExp
      * blockID_Reference2 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID]);

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
      IndirectLoop::Fortran::PlanFunction::VariableNames::poffset,
      subroutineScope);

  SgVarRefExp * offset_b_Reference3 = buildOpaqueVarRefExp (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::offset_b,
      subroutineScope);

  SgVarRefExp
      * blockID_Reference3 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID]);

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
      CUDA::Fortran::VariableNames::blockdim, subroutineScope);

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
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcol,
      subroutineScope);

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

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * ind_argSize_Reference = buildVarRefExp (
          localVariables_indArgSizes[i]);

      SgVarRefExp
          * pindSizes_Reference4 =
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizes]);

      SgVarRefExp
          * blockID_Reference4 =
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID]);

      SgAddOp * arrayIndexExpression = buildAddOp (buildIntVal (
          pindSizesArrayOffset), buildMultiplyOp (blockID_Reference4,
          buildIntVal (
              parallelLoop->getNumberOfDistinctIndirect_OP_DAT_Arguments ())));

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
FortranCUDAKernelSubroutineIndirectLoop::createPlanlocalVariableDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  vector <string> fourByteIntegerArrayVariables;

  fourByteIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizes);

  fourByteIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffs);

  fourByteIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMap);

  fourByteIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::poffset);

  fourByteIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnelems);

  fourByteIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcol);

  fourByteIntegerArrayVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcol);

  for (vector <string>::const_iterator it =
      fourByteIntegerArrayVariables.begin (); it
      != fourByteIntegerArrayVariables.end (); ++it)
  {
    SgIntVal * lowerBoundExpression = buildIntVal (0);

    SgVarRefExp
        * argsSizesReference =
            buildVarRefExp (
                variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]);

    SgVarRefExp * fieldReference = buildVarRefExp (
        dataSizesDeclaration->getFieldDeclaration (*it + "Size"));

    SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
        fieldReference);

    SgSubtractOp * upperBoundExpression = buildSubtractOp (
        fieldSelectionExpression, buildIntVal (1));

    variableDeclarations[*it]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            *it, FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                FortranTypesBuilder::getFourByteInteger (),
                lowerBoundExpression, upperBoundExpression), subroutineScope,
            formalParameters, 1, DEVICE);
  }

  vector <string> fourByteIntegerVariables;

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::blockOffset);

  for (vector <string>::const_iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations[*it]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            *it, FortranTypesBuilder::getFourByteInteger (), subroutineScope,
            formalParameters, 1, VALUE);
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::create_OP_DAT_localVariableDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters", 2);

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp
          * argsSizesReference =
              buildVarRefExp (
                  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]);

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclaration->getFieldDeclaration (
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

      SgType * opdatType = parallelLoop->get_OP_DAT_Type (i);

      SgArrayType * baseArrayType = isSgArrayType (opdatType);

      ROSE_ASSERT ( baseArrayType != NULL );

      variableDeclarations[VariableNames::getOpDatName (i)]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getOpDatName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  baseArrayType->get_base_type (), lowerBoundExpression,
                  upperBoundExpression), subroutineScope, formalParameters, 1,
              DEVICE);

      SgIntVal * lowerBoundExpression2 = buildIntVal (0);

      SgVarRefExp
          * argsSizesReference2 =
              buildVarRefExp (
                  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]);

      SgVarRefExp * fieldReference2 = buildVarRefExp (
          dataSizesDeclaration->getFieldDeclaration (
              VariableNames::getLocalToGlobalMappingSizeName (i)));

      SgDotExp * fieldSelectionExpression2 = buildDotExp (argsSizesReference2,
          fieldReference2);

      SgSubtractOp * upperBoundExpression2 = buildSubtractOp (
          fieldSelectionExpression2, buildIntVal (1));

      variableDeclarations[VariableNames::getLocalToGlobalMappingName (i)]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getLocalToGlobalMappingName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  FortranTypesBuilder::getFourByteInteger (),
                  lowerBoundExpression2, upperBoundExpression2),
              subroutineScope, formalParameters, 1, DEVICE);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp
          * argsSizesReference =
              buildVarRefExp (
                  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]);

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclaration->getFieldDeclaration (
              VariableNames::getGlobalToLocalMappingSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
          fieldReference);

      SgSubtractOp * upperBoundExpression = buildSubtractOp (
          fieldSelectionExpression, buildIntVal (1));

      variableDeclarations[VariableNames::getGlobalToLocalMappingName (i)]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getGlobalToLocalMappingName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  FortranTypesBuilder::getTwoByteInteger (),
                  lowerBoundExpression, upperBoundExpression), subroutineScope,
              formalParameters, 1, DEVICE);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == DIRECT)
    {
      string const variableName = VariableNames::getOpDatName (i);

      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp
          * argsSizesReference =
              buildVarRefExp (
                  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]);

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclaration->getFieldDeclaration (
              VariableNames::getOpDatSizeName (i)));

      SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
          fieldReference);

      SgSubtractOp * upperBoundExpression = buildSubtractOp (
          fieldSelectionExpression, buildIntVal (1));

      variableDeclarations[VariableNames::getOpDatName (i)]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getOpDatName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  parallelLoop->get_OP_DAT_Type (i), lowerBoundExpression,
                  upperBoundExpression), subroutineScope, formalParameters, 1,
              DEVICE);
    }
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createStatements ()
{
  createThreadZeroStatements ();

  initialiseLocalVariables ();

  createAutosharedWhileLoops ();

  createPlanWhileLoop ();

  applyPointeredIncrementsOrWrites ();
}

void
FortranCUDAKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  createLocalThreadDeclarations ();

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    string const & variableName = VariableNames::getNumberOfBytesVariableName (
        i);

    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      variableDeclarations[variableName]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);

      positionOf_nbytes[parallelLoop->get_OP_DAT_VariableName (i)] = i;
    }
    else
    {
      variableDeclarations[variableName]
          = variableDeclarations[VariableNames::getNumberOfBytesVariableName (
              positionOf_nbytes[parallelLoop->get_OP_DAT_VariableName (i)])];
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      string const & variableName = VariableNames::getRoundUpVariableName (i);

      variableDeclarations[variableName]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_Access_Value (i) == INC_ACCESS)
    {
      string const variableName = "arg" + lexical_cast <string> (i) + "_map";

      localVariables_incrementAccessMaps[i]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "ind_arg" + lexical_cast <string> (i)
          + "_size";

      localVariables_indArgSizes[i]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope, 1, SHARED);
    }
  }

  vector <string> sharedFourByteIntegerVariables;

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::blockID);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::ncolor);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::nelem);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::nelems2);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::offset_b);

  for (vector <string>::const_iterator it =
      sharedFourByteIntegerVariables.begin (); it
      != sharedFourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations[*it]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            *it, FortranTypesBuilder::getFourByteInteger (), subroutineScope,
            1, SHARED);
  }

  SgExpression * upperBoundExpression = new SgAsteriskShapeExp (
      ROSEHelper::getFileInfo ());

  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::VariableNames::autoshared,
          FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getDoublePrecisionFloat (), 0,
              upperBoundExpression), subroutineScope, 1, SHARED);

  vector <string> fourByteIntegerVariables;
  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::col);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::col2);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::iterationCounter2);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduloResult);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::moduled);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::nbytes);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::KernelSubroutine::VariableNames::whileLoopBound);

  for (vector <string>::const_iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations[*it]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            *it, FortranTypesBuilder::getFourByteInteger (), subroutineScope);
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createlocalVariableDeclarations ()
{
  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          IndirectAndDirectLoop::Fortran::VariableNames::argsSizes,
          dataSizesDeclaration->getType (), subroutineScope, formalParameters,
          1, DEVICE);

  create_OP_DAT_localVariableDeclarations ();

  createPlanlocalVariableDeclarations ();
}

FortranCUDAKernelSubroutineIndirectLoop::FortranCUDAKernelSubroutineIndirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclaration) :
  FortranCUDAKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope)
{
  this->dataSizesDeclaration = dataSizesDeclaration;

  createlocalVariableDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}