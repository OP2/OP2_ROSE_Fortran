#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <KernelSubroutineOfIndirectLoop.h>
#include <CommonNamespaces.h>
#include <ROSEHelper.h>

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

namespace
{
  /*
   * ======================================================
   * Following are names of local shared variables needed
   * in the kernel subroutine of an indirect loop
   * ======================================================
   */

  std::string const variableName_blockID = "blockID";
  std::string const variableName_col = "col";
  std::string const variableName_col2 = "col2";
  std::string const variableName_iterationCounter = "i";
  std::string const variableName_iterationCounter2 = "i2";
  std::string const variableName_moduled = "moduled";
  std::string const variableName_moduloResult = "moduloResult";
  std::string const variableName_nbytes = "nbytes";
  std::string const variableName_ncolor = "ncolor";
  std::string const variableName_nelem = "nelem";
  std::string const variableName_nelems2 = "nelems2";
  std::string const variableName_offset_b = "offset_b";
  std::string const variableName_whileLoopBound = "whileLoopBound";
}

void
KernelSubroutineOfIndirectLoop::createPlanWhileLoop (
    UserDeviceSubroutine & userDeviceSubroutine, ParallelLoop & parallelLoop)
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
  using SageInterface::appendStatement;

  SgVarRefExp * iterationCounter_Reference1 = buildVarRefExp (
      localVariables_Others[variableName_iterationCounter]);

  SgVarRefExp * variable_X_Reference1 = buildOpaqueVarRefExp (variableName_x,
      subroutineScope);

  SgVarRefExp * variable_Threadidx_Reference1 = buildOpaqueVarRefExp (
      variableName_threadidx, subroutineScope);

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
      localVariables_Others[variableName_col2]), buildIntVal (-1));

  loopBodyNestingLevel1->append_statement (buildExprStatement (
      loopBodyExpression1));

  SgExpression * ifGuardNestingLevel1 = buildLessThanOp (buildVarRefExp (
      localVariables_Others[variableName_iterationCounter]), buildVarRefExp (
      localVariables_Others[variableName_nelem]));

  SgBasicBlock * ifBodyNestingLevel1 = buildBasicBlock ();

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_Access_Value (i) == INC_ACCESS)
    {
      SgAssignOp * loopLowerBoundNestingLevel2 = buildAssignOp (buildVarRefExp (
          localVariables_Others[variableName_iterationCounter2]), buildIntVal (
          0));

      SgIntVal * loopUpperBoundNestingLevel2 = buildIntVal (
          parallelLoop.get_OP_DAT_Dimension (i) - 1);

      SgBasicBlock * loopBodyNestingLevel2 = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
          localVariables_localThreadVariables[i]), buildVarRefExp (
          localVariables_Others[variableName_iterationCounter2]));

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

  //  SgFunctionSymbol * userSubroutineSymbol =
  //      FortranTypesBuilder::buildNewFortranSubroutine (
  //          userDeviceSubroutine.getSubroutineName (), subroutineScope);

  //  SgExprListExp * userSubroutineActualParameters = buildExprListExp ();

  //  SgFunctionCallExp * userSubroutineCall = buildFunctionCallExp (
  //      userSubroutineSymbol, userSubroutineActualParameters);

  //  for (unsigned int i = 1; i
  //      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  //  {
  //		int dim = parallelLoop.get_OP_DAT_Dimension ( i );
  //		if ( parallelLoop.get_OP_MAP_Value (i) == GLOBAL )
  //		{
  //			if ( parallelLoop.get_OP_Access_Value (i) == READ_ACCESS )
  //				SgVarRefExp * autoshared_Reference = buildVarRefExp (
  //					localVariables_Others[kernelSharedVariables::variableName_autoshared]);
  //
  //			else
  //				aaa
  //		}
  //		else if
  //
  //
  //
  //    if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
  //    {
  //      if (parallelLoop.get_OP_Access_Value (i) == INC_ACCESS)
  //      {
  //        userSubroutineActualParameters->append_expression (buildVarRefExp (
  //            localVariables_incrementAccessArrays[i]));
  //      }
  //      else
  //      {
  //        SgVarRefExp * autoshared_Reference = buildVarRefExp (
  //					localVariables_Others[kernelSharedVariables::variableName_autoshared]);
  //
  //        SgVarRefExp * globalToLocalMappingArray_Reference = buildVarRefExp (
  //            formalParameters_GlobalToLocalMapping[i]);
  //
  //        SgVarRefExp * nbytes_Reference = buildVarRefExp (
  //            localVariables_nbytes[i]);
  //
  //        SgAddOp * lowerBound_addExpression1 = buildAddOp (buildVarRefExp (
  //            localVariables_Others[variableName_iterationCounter]),
  //            buildVarRefExp (localVariables_Others[variableName_offset_b]));
  //
  //        SgPntrArrRefExp * lowerBound_arrayExpression = buildPntrArrRefExp (
  //            globalToLocalMappingArray_Reference, lowerBound_addExpression1);
  //
  //        SgMultiplyOp * lowerBound_multiplyExpression = buildMultiplyOp (
  //            lowerBound_arrayExpression, buildIntVal (
  //                parallelLoop.get_OP_DAT_Dimension (i)));
  //
  //        SgAddOp * lowerBound_addExpression2 = buildAddOp (nbytes_Reference,
  //            lowerBound_multiplyExpression);
  //
  //        SgAddOp * upperBound_addExpression1 = buildAddOp (buildVarRefExp (
  //            localVariables_Others[variableName_iterationCounter]),
  //            buildVarRefExp (localVariables_Others[variableName_offset_b]));
  //
  //        SgPntrArrRefExp * upperBound_arrayExpression = buildPntrArrRefExp (
  //            globalToLocalMappingArray_Reference, upperBound_addExpression1);
  //
  //        SgMultiplyOp * upperBound_multiplyExpression = buildMultiplyOp (
  //            upperBound_arrayExpression, buildIntVal (
  //                parallelLoop.get_OP_DAT_Dimension (i)));
  //
  //        SgAddOp * upperBound_addExpression2 = buildAddOp (nbytes_Reference,
  //            upperBound_multiplyExpression);
  //
  //        SgAddOp * upperBound_addExpression3 = buildAddOp (
  //            upperBound_addExpression2, buildIntVal (
  //                parallelLoop.get_OP_DAT_Dimension (i)));
  //
  //        SgSubscriptExpression * subscriptExpression =
  //            new SgSubscriptExpression (ROSEHelper::getFileInfo (),
  //                lowerBound_addExpression2, upperBound_addExpression3,
  //                buildIntVal (1));
  //        subscriptExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());
  //
  //        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
  //            autoshared_Reference, buildExprListExp (subscriptExpression));
  //
  //        userSubroutineActualParameters->append_expression (arrayExpression);
  //      }
  //    }
  //  }

  SgStatement * userSubroutineCall = createUserSubroutineCall (
      userDeviceSubroutine,
      localVariables_Others[variableName_iterationCounter],
      localVariables_Others[variableName_offset_b], parallelLoop,
      &formalParameters_GlobalToLocalMapping, &localVariables_nbytes);

  ifBodyNestingLevel1->append_statement (userSubroutineCall);

  SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
      localVariables_Others[variableName_iterationCounter]), buildVarRefExp (
      localVariables_Others[variableName_offset_b]));

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
      formalParameters_PlanVariables[PlanFunctionVariables::pthrcol]),
      addExpression2);

  SgAssignOp * assignmentExpression2 = buildAssignOp (buildVarRefExp (
      localVariables_Others[variableName_col2]), arrayExpression2);

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
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT
        && parallelLoop.get_OP_Access_Value (i) == INC_ACCESS)
    {
      SgVarRefExp * incrementAccessMap_Reference = buildVarRefExp (
          localVariables_incrementAccessMaps[i]);

      SgVarRefExp * globalToLocalMappingArray_Reference = buildVarRefExp (
          formalParameters_GlobalToLocalMapping[i]);

      SgAddOp * addExpression = buildAddOp (buildVarRefExp (
          localVariables_Others[variableName_iterationCounter]),
          buildVarRefExp (localVariables_Others[variableName_offset_b]));

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
      localVariables_Others[variableName_col2]), buildVarRefExp (
      localVariables_Others[variableName_col]));

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT
        && parallelLoop.get_OP_Access_Value (i) == INC_ACCESS)
    {
      SgBasicBlock * loopBodyNestingLevel3 = buildBasicBlock ();

      SgVarRefExp
          * autoshared_Reference_LHS =
              buildVarRefExp (
                  localVariables_Others[kernelSharedVariables::autoShared]);

      SgVarRefExp
          * autoshared_Reference_RHS =
              buildVarRefExp (
                  localVariables_Others[kernelSharedVariables::autoShared]);

      SgVarRefExp * incrementAccessArrays_Reference = buildVarRefExp (
          localVariables_localThreadVariables[i]);

      SgVarRefExp * incrementAccessMap_Reference = buildVarRefExp (
          localVariables_incrementAccessMaps[i]);

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          incrementAccessArrays_Reference, buildVarRefExp (
              localVariables_Others[variableName_iterationCounter2]));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          incrementAccessMap_Reference, buildIntVal (
              parallelLoop.get_OP_DAT_Dimension (i)));

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          localVariables_Others[variableName_iterationCounter2]),
          multiplyExpression);

      SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
          localVariables_nbytes[i]), addExpression1);

      SgPntrArrRefExp * rhsExpression = buildPntrArrRefExp (
          autoshared_Reference_RHS, addExpression2);

      SgPntrArrRefExp * lhsExpression = buildPntrArrRefExp (
          autoshared_Reference_LHS, addExpression2);

      SgAssignOp * assignmentExpression = buildAssignOp (lhsExpression,
          buildAddOp (rhsExpression, arrayExpression1));

      loopBodyNestingLevel3->append_statement (buildExprStatement (
          assignmentExpression));

      SgAssignOp * loopLowerBoundNestingLevel3 = buildAssignOp (buildVarRefExp (
          localVariables_Others[variableName_iterationCounter2]), buildIntVal (
          0));

      SgIntVal * loopUpperBoundNestingLevel3 = buildIntVal (
          parallelLoop.get_OP_DAT_Dimension (i) - 1);

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

  SgAssignOp * loopLowerBoundNestingLevel2 = buildAssignOp (buildVarRefExp (
      localVariables_Others[variableName_col]), buildIntVal (0));

  SgSubtractOp * loopUpperBoundNestingLevel2 = buildSubtractOp (buildVarRefExp (
      localVariables_Others[variableName_ncolor]), buildIntVal (1));

  SgFortranDo * loopStatementNestingLevel2 =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          loopLowerBoundNestingLevel2, loopUpperBoundNestingLevel2,
          buildIntVal (1), loopBodyNestingLevel2);

  loopBodyNestingLevel1->append_statement (loopStatementNestingLevel2);

  /*
   * ======================================================
   * While loop guard
   * ======================================================
   */

  SgExpression * loopGuardNestingLevel1 = buildLessThanOp (buildVarRefExp (
      localVariables_Others[variableName_iterationCounter]), buildVarRefExp (
      localVariables_Others[variableName_nelems2]));

  SgWhileStmt * loopStatementNestingLevel1 = buildWhileStmt (
      loopGuardNestingLevel1, loopBodyNestingLevel1);

  loopStatementNestingLevel1->set_has_end_statement (true);

  appendStatement (loopStatementNestingLevel1, subroutineScope);
}

void
KernelSubroutineOfIndirectLoop::createAutosharedWhileLoops (
    ParallelLoop & parallelLoop)
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
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      /*
       * ======================================================
       * Initialise the lower bound of the while loop
       * ======================================================
       */

      SgVarRefExp * iterationCounter_Reference1 = buildVarRefExp (
          localVariables_Others[variableName_iterationCounter]);

      SgVarRefExp * variable_X_Reference1 = buildOpaqueVarRefExp (
          variableName_x, subroutineScope);

      SgVarRefExp * variable_Threadidx_Reference1 = buildOpaqueVarRefExp (
          variableName_threadidx, subroutineScope);

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

      SgVarRefExp * whileLoopBound_Reference2 = buildVarRefExp (
          localVariables_Others[variableName_whileLoopBound]);

      SgVarRefExp * indArgSize_Reference2 = buildVarRefExp (
          localVariables_indArgSizes[i]);

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
          indArgSize_Reference2, buildIntVal (
              parallelLoop.get_OP_DAT_Dimension (i)));

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

      if (parallelLoop.get_OP_Access_Value (i) != INC_ACCESS)
      {

        SgVarRefExp * iterationCounter_Reference3 = buildVarRefExp (
            localVariables_Others[variableName_iterationCounter]);

        SgVarRefExp * moduled_Reference3 = buildVarRefExp (
            localVariables_Others[variableName_moduled]);

        SgFunctionSymbol * modFunctionSymbol =
            FortranTypesBuilder::buildNewFortranFunction ("mod",
                subroutineScope);

        SgExprListExp * modActualParameters = buildExprListExp (
            iterationCounter_Reference3, buildIntVal (
                parallelLoop.get_OP_DAT_Dimension (i)));

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
            * autoShared_Reference4 =
                buildVarRefExp (
                    localVariables_Others[kernelSharedVariables::autoShared]);

        SgVarRefExp * iterationCounter_Reference4 = buildVarRefExp (
            localVariables_Others[variableName_iterationCounter]);

        SgVarRefExp * nbytes_Reference4 = buildVarRefExp (
            localVariables_nbytes[i]);

        SgAddOp * addExpression4 = buildAddOp (nbytes_Reference4,
            iterationCounter_Reference4);

        SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (
            autoShared_Reference4, addExpression4);

        SgVarRefExp * blockId_Reference4 = buildVarRefExp (
            localVariables_Others[variableName_blockID]);

        SgVarRefExp * pindOffs_Reference4 = buildVarRefExp (
            formalParameters_PlanVariables[PlanFunctionVariables::pindOffs]);

        SgMultiplyOp * multiplyExpression4_a = buildMultiplyOp (
            blockId_Reference4, buildIntVal (4));

        SgAddOp * addExpression4_a = buildAddOp (buildIntVal (pindOffsOffset),
            multiplyExpression4_a);

        SgPntrArrRefExp * arrayIndexExpression4_a = buildPntrArrRefExp (
            pindOffs_Reference4, addExpression4_a);

        SgVarRefExp * iterationCounter_Reference4_a = buildVarRefExp (
            localVariables_Others[variableName_iterationCounter]);

        SgDivideOp * divideExpression4 = buildDivideOp (
            iterationCounter_Reference4_a, buildIntVal (
                parallelLoop.get_OP_DAT_Dimension (i)));

        SgAddOp * addExpression4_b = buildAddOp (arrayIndexExpression4_a,
            divideExpression4);

        SgAddOp * addExpression4_c = buildAddOp (buildIntVal (0),
            addExpression4_b);

        SgVarRefExp * pindMaps_Reference4 = buildVarRefExp (
            formalParameters_LocalToGlobalMapping[i]);

        SgPntrArrRefExp * arrayIndexExpression4_b = buildPntrArrRefExp (
            pindMaps_Reference4, addExpression4_c);

        SgMultiplyOp * multiplyExpression4_b = buildMultiplyOp (
            arrayIndexExpression4_b, buildIntVal (
                parallelLoop.get_OP_DAT_Dimension (i)));

        SgVarRefExp * moduled_Reference = buildVarRefExp (
            localVariables_Others[variableName_moduled]);

        SgAddOp * addExpression4_d = buildAddOp (moduled_Reference,
            multiplyExpression4_b);

        SgVarRefExp * OP_DAT_Reference4 = buildVarRefExp (
            formalParameter_OP_DATs[i]);

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
            * autoShared_Reference4 =
                buildVarRefExp (
                    localVariables_Others[kernelSharedVariables::autoShared]);

        SgVarRefExp * iterationCounter_Reference4 = buildVarRefExp (
            localVariables_Others[variableName_iterationCounter]);

        SgVarRefExp * nbytes_Reference4 = buildVarRefExp (
            localVariables_nbytes[i]);

        SgAddOp * addExpression4 = buildAddOp (nbytes_Reference4,
            iterationCounter_Reference4);

        SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (
            autoShared_Reference4, addExpression4);

        SgAssignOp * assignmentExpression4 = buildAssignOp (
            arrayIndexExpression4, buildIntVal (0));

        loopBody->append_statement (buildExprStatement (assignmentExpression4));
      }

      /*
       * ======================================================
       * Statement to increment loop counter
       * ======================================================
       */

      SgVarRefExp * iterationCounter_Reference5_LHS = buildVarRefExp (
          localVariables_Others[variableName_iterationCounter]);

      SgVarRefExp * iterationCounter_Reference5_RHS = buildVarRefExp (
          localVariables_Others[variableName_iterationCounter]);

      SgVarRefExp * variable_X_Reference5 = buildOpaqueVarRefExp (
          variableName_x, subroutineScope);

      SgVarRefExp * variable_BlockDim_Reference5 = buildOpaqueVarRefExp (
          variableName_blockdim, subroutineScope);

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

      SgVarRefExp * iterationCounter_Reference6 = buildVarRefExp (
          localVariables_Others[variableName_iterationCounter]);

      SgVarRefExp * whileLoopBound_Reference6 = buildVarRefExp (
          localVariables_Others[variableName_whileLoopBound]);

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
KernelSubroutineOfIndirectLoop::initialiseLocalVariables (
    ParallelLoop & parallelLoop)
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
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * inRoundUp_Reference = buildVarRefExp (
          localVariables_inRoundUps[i]);

      SgVarRefExp * indArgSize_Reference = buildVarRefExp (
          localVariables_indArgSizes[i]);

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          indArgSize_Reference, buildIntVal (parallelLoop.get_OP_DAT_Dimension (
              i)));

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
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      if (firstInitialization)
      {
        firstInitialization = false;

        SgVarRefExp * nbytes_Reference = buildVarRefExp (
            localVariables_nbytes[i]);

        SgAssignOp * assignmentExpression = buildAssignOp (nbytes_Reference,
            buildIntVal (0));

        appendStatement (buildExprStatement (assignmentExpression),
            subroutineScope);
      }
      else
      {
        SgVarRefExp * nbytes_Reference = buildVarRefExp (
            localVariables_nbytes[i]);

        SgVarRefExp * previous_nbytes_Reference = buildVarRefExp (
            localVariables_nbytes[previous_OP_DAT_Location]);

        SgVarRefExp * previous_inRoundUp_Reference = buildVarRefExp (
            localVariables_inRoundUps[previous_OP_DAT_Location]);

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
KernelSubroutineOfIndirectLoop::createThreadZeroStatements (
    ParallelLoop & parallelLoop)
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
      variableName_threadidx, subroutineScope);

  SgVarRefExp * x_Reference = buildOpaqueVarRefExp (variableName_x,
      subroutineScope);

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildDotExp (
      threadidx_Reference, x_Reference), buildIntVal (0));

  /*
   * ======================================================
   * 1st statement
   * ======================================================
   */

  SgVarRefExp * blockID_Reference1 = buildVarRefExp (
      localVariables_Others[variableName_blockID]);

  SgVarRefExp * blockOffset_Reference1 = buildVarRefExp (
      formalParameters_PlanVariables[PlanFunctionVariables::blockOffset]);

  SgVarRefExp * pblkMap_Reference1 = buildVarRefExp (
      formalParameters_PlanVariables[PlanFunctionVariables::pblkMap]);

  SgVarRefExp * blockidx_Reference1 = buildOpaqueVarRefExp (
      variableName_blockidx, subroutineScope);

  SgVarRefExp * x_Reference1 = buildOpaqueVarRefExp (variableName_x,
      subroutineScope);

	SgDotExp * blockIdxDotX = buildDotExp ( blockidx_Reference1, x_Reference1 );

  SgSubtractOp * subtractExpression1 = buildSubtractOp ( blockIdxDotX, buildIntVal (1) );

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
      PlanFunctionVariables::pnelems, subroutineScope);

  SgVarRefExp * nelem_Reference2 = buildOpaqueVarRefExp (variableName_nelem,
      subroutineScope);

  SgVarRefExp * blockID_Reference2 = buildVarRefExp (
      localVariables_Others[variableName_blockID]);

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
      PlanFunctionVariables::poffset, subroutineScope);

  SgVarRefExp * offset_b_Reference3 = buildOpaqueVarRefExp (
      variableName_offset_b, subroutineScope);

  SgVarRefExp * blockID_Reference3 = buildVarRefExp (
      localVariables_Others[variableName_blockID]);

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
		variableName_nelems2, subroutineScope);

	
	SgVarRefExp * blockdim_Reference1 = buildOpaqueVarRefExp (
		variableName_blockdim, subroutineScope);
		
	SgDotExp * blockdimxDotX = buildDotExp ( blockdim_Reference1, x_Reference1 );

	SgExpression * nelem2InitSubExpr = buildAddOp ( buildIntVal ( 1 ),
		buildDivideOp ( buildSubtractOp ( nelem_Reference2, buildIntVal ( 1 ) ),
		  blockdimxDotX ) );
	
	SgExpression * nelem2InitExpr = buildMultiplyOp ( blockdimxDotX , nelem2InitSubExpr );
	
	SgStatement * initNelems2 = buildAssignStatement ( pNelems2_Reference3,
		nelem2InitExpr );
	
	ifBlock->append_statement ( initNelems2 );

	
	/*
   * ======================================================
   * 5th statement: assignment of ncolor
   * ======================================================
   */
	
	SgVarRefExp * pNcolor_Reference3 = buildOpaqueVarRefExp (
		variableName_ncolor, subroutineScope);

	SgVarRefExp * pnthrcol_Reference = buildOpaqueVarRefExp (
		PlanFunctionVariables::pnthrcol, subroutineScope);

	
	SgPntrArrRefExp * pncolorsOfBlockid = buildPntrArrRefExp ( pnthrcol_Reference ,
		blockID_Reference3);

	
	SgStatement * initNcolor = buildAssignStatement ( pNcolor_Reference3,
		pncolorsOfBlockid );
	
	ifBlock->append_statement ( initNcolor );	

  /*
   * ======================================================
   * assignment of ind_arg_size variables
   * ======================================================
   */

  unsigned int pindSizesArrayOffset = 0;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i )
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * ind_argSize_Reference = buildVarRefExp (
          localVariables_indArgSizes[i]);

      SgVarRefExp * pindSizes_Reference4 = buildVarRefExp (
          formalParameters_PlanVariables[PlanFunctionVariables::pindSizes]);

      SgVarRefExp * blockID_Reference4 = buildVarRefExp (
          localVariables_Others[variableName_blockID]);

      SgAddOp * arrayIndexExpression = buildAddOp (buildIntVal (
          pindSizesArrayOffset), buildMultiplyOp (blockID_Reference4,
          buildIntVal (
              parallelLoop.getNumberOfDistinctIndirect_OP_DAT_Arguments ())));

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
KernelSubroutineOfIndirectLoop::createRemainingLocalVariables ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  vector <string> fourByteIntegerVariables;
  fourByteIntegerVariables.push_back (variableName_col);
  fourByteIntegerVariables.push_back (variableName_col2);
  fourByteIntegerVariables.push_back (variableName_iterationCounter);
  fourByteIntegerVariables.push_back (variableName_iterationCounter2);
  fourByteIntegerVariables.push_back (variableName_moduloResult);
  fourByteIntegerVariables.push_back (variableName_moduled);
  fourByteIntegerVariables.push_back (variableName_nbytes);
  fourByteIntegerVariables.push_back (variableName_whileLoopBound);

  for (vector <string>::const_iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
        *it, FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

    variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (variableDeclaration, subroutineScope);

    localVariables_Others[*it] = variableDeclaration;
  }
}

void
KernelSubroutineOfIndirectLoop::createLocalVariables (
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      string const variableName = "nbytes" + lexical_cast <string> (i);

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variableDeclaration, subroutineScope);

      localVariables_nbytes[i] = variableDeclaration;

      positionOf_nbytes[parallelLoop.get_OP_DAT_VariableName (i)] = i;
    }
    else
    {
      localVariables_nbytes[i]
          = localVariables_nbytes[positionOf_nbytes[parallelLoop.get_OP_DAT_VariableName (
              i)]];

    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      string const variableName = "inRoundUp" + lexical_cast <string> (i);

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variableDeclaration, subroutineScope);

      localVariables_inRoundUps[i] = variableDeclaration;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_Access_Value (i) == INC_ACCESS)
    {
      string const variableName = "arg" + lexical_cast <string> (i) + "_map";

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variableDeclaration, subroutineScope);

      localVariables_incrementAccessMaps[i] = variableDeclaration;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_Access_Value (i) == INC_ACCESS)
    {
      string const variableName = "arg" + lexical_cast <string> (i) + "_l";

      SgIntVal * lowerBound = buildIntVal (0);

      SgIntVal * upperBound = buildIntVal (
          parallelLoop.get_OP_DAT_Dimension (i) - 1);

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              parallelLoop.get_OP_DAT_Type (i), lowerBound, upperBound), NULL,
          subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variableDeclaration, subroutineScope);

      localVariables_localThreadVariables[i] = variableDeclaration;

      //      localVariables_incrementAccessArrays[i] = variableDeclaration;
    }
  }
}

void
KernelSubroutineOfIndirectLoop::createSharedLocalVariables (
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "ind_arg" + lexical_cast <string> (i)
          + "_size";

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
      variableDeclaration->get_declarationModifier ().get_typeModifier ().setShared ();

      appendStatement (variableDeclaration, subroutineScope);

      localVariables_indArgSizes[i] = variableDeclaration;
    }
  }

  vector <string> sharedFourByteIntegerVariables;
  sharedFourByteIntegerVariables.push_back (variableName_blockID);
  sharedFourByteIntegerVariables.push_back (variableName_ncolor);
  sharedFourByteIntegerVariables.push_back (variableName_nelem);
  sharedFourByteIntegerVariables.push_back (variableName_nelems2);
  sharedFourByteIntegerVariables.push_back (variableName_offset_b);

  for (vector <string>::const_iterator it =
      sharedFourByteIntegerVariables.begin (); it
      != sharedFourByteIntegerVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
        *it, FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

    variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
		variableDeclaration->get_declarationModifier ().get_typeModifier ().setShared ();

    appendStatement (variableDeclaration, subroutineScope);

    localVariables_Others[*it] = variableDeclaration;
  }

  SgExpression * upperBoundExpression = new SgAsteriskShapeExp (
      ROSEHelper::getFileInfo ());

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      kernelSharedVariables::autoShared,
      FortranTypesBuilder::getArray_RankOne (
          FortranTypesBuilder::getDoublePrecisionFloat (), 0,
          upperBoundExpression), NULL, subroutineScope);

  variableDeclaration->get_declarationModifier ().get_typeModifier ().setShared ();
  variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration, subroutineScope);

  localVariables_Others[kernelSharedVariables::autoShared]
      = variableDeclaration;
}

void
KernelSubroutineOfIndirectLoop::createPlanFormalParameters (
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  vector <string> fourByteIntegerArrayPlanVariables;
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pindSizes);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pindOffs);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pblkMap);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::poffset);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pnelems);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pnthrcol);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pthrcol);

  for (vector <string>::const_iterator it =
      fourByteIntegerArrayPlanVariables.begin (); it
      != fourByteIntegerArrayPlanVariables.end (); ++it)
  {
    SgIntVal * lowerBoundExpression = buildIntVal (0);

    SgVarRefExp * argsSizesReference = buildVarRefExp (
        formalParameter_argsSizes);

    SgVarRefExp * fieldReference = buildVarRefExp (
        deviceDataSizesDeclaration.getPlanVariableSizeFieldDeclaration (*it
            + "Size"));

    SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
        fieldReference);

    SgSubtractOp * upperBoundExpression = buildSubtractOp (
        fieldSelectionExpression, buildIntVal (1));

    SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
        *it, FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
            FortranTypesBuilder::getFourByteInteger (), lowerBoundExpression,
            upperBoundExpression), NULL, subroutineScope);

    variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
    variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();

    appendStatement (variableDeclaration, subroutineScope);

    formalParameters->append_arg (
        *(variableDeclaration->get_variables ().begin ()));

    formalParameters_PlanVariables[*it] = variableDeclaration;
  }

  vector <string> fourByteIntegerPlanVariables;
  fourByteIntegerPlanVariables.push_back (PlanFunctionVariables::blockOffset);

  for (vector <string>::const_iterator it =
      fourByteIntegerPlanVariables.begin (); it
      != fourByteIntegerPlanVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
        *it, FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

    variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
    variableDeclaration->get_declarationModifier ().get_typeModifier (). setValue ();

    appendStatement (variableDeclaration, subroutineScope);

    formalParameters->append_arg (
        *(variableDeclaration->get_variables ().begin ()));

    formalParameters_PlanVariables[*it] = variableDeclaration;
  }
}

void
KernelSubroutineOfIndirectLoop::create_OP_DAT_FormalParameters (
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters", 2);

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = OP_DAT_ArgumentNames::OP_DAT_NamePrefix
          + lexical_cast <string> (i);

      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp * argsSizesReference = buildVarRefExp (
          formalParameter_argsSizes);

      SgVarRefExp * fieldReference = buildVarRefExp (
          deviceDataSizesDeclaration.get_OP_DAT_SizeFieldDeclaration (i));

      SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
          fieldReference);

      SgSubtractOp * upperBoundExpression = buildSubtractOp (
          fieldSelectionExpression, buildIntVal (1));

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              parallelLoop.get_OP_DAT_Type (i), lowerBoundExpression,
              upperBoundExpression), NULL, subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
      variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();

      appendStatement (variableDeclaration, subroutineScope);

      formalParameters->append_arg (
          *(variableDeclaration->get_variables ().begin ()));

      formalParameter_OP_DATs[i] = variableDeclaration;

      string const variableName2 = "pindMaps" + lexical_cast <string> (i);

      SgIntVal * lowerBoundExpression2 = buildIntVal (0);

      SgVarRefExp * argsSizesReference2 = buildVarRefExp (
          formalParameter_argsSizes);

      SgVarRefExp
          * fieldReference2 =
              buildVarRefExp (
                  deviceDataSizesDeclaration.get_LocalToGlobalMappingSizeFieldDeclaration (
                      i));

      SgDotExp * fieldSelectionExpression2 = buildDotExp (argsSizesReference2,
          fieldReference2);

      SgSubtractOp * upperBoundExpression2 = buildSubtractOp (
          fieldSelectionExpression2, buildIntVal (1));

      SgVariableDeclaration * variableDeclaration2 = buildVariableDeclaration (
          variableName2,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getFourByteInteger (),
              lowerBoundExpression2, upperBoundExpression2), NULL,
          subroutineScope);

      variableDeclaration2->get_declarationModifier ().get_accessModifier ().setUndefined ();
      variableDeclaration2->get_declarationModifier ().get_typeModifier ().setDevice ();

      appendStatement (variableDeclaration2, subroutineScope);

      formalParameters->append_arg (
          *(variableDeclaration2->get_variables ().begin ()));

      formalParameters_LocalToGlobalMapping[i] = variableDeclaration2;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "pMaps" + lexical_cast <string> (i);

      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp * argsSizesReference = buildVarRefExp (
          formalParameter_argsSizes);

      SgVarRefExp
          * fieldReference =
              buildVarRefExp (
                  deviceDataSizesDeclaration.get_GlobalToLocalMappingSizeFieldDeclaration (
                      i));

      SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
          fieldReference);

      SgSubtractOp * upperBoundExpression = buildSubtractOp (
          fieldSelectionExpression, buildIntVal (1));

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getTwoByteInteger (), lowerBoundExpression,
              upperBoundExpression), NULL, subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
      variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();

      appendStatement (variableDeclaration, subroutineScope);

      formalParameters->append_arg (
          *(variableDeclaration->get_variables ().begin ()));

      formalParameters_GlobalToLocalMapping[i] = variableDeclaration;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == DIRECT)
    {
      string const variableName = OP_DAT_ArgumentNames::OP_DAT_NamePrefix
          + lexical_cast <string> (i);

      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp * argsSizesReference = buildVarRefExp (
          formalParameter_argsSizes);

      SgVarRefExp * fieldReference = buildVarRefExp (
          deviceDataSizesDeclaration.get_OP_DAT_SizeFieldDeclaration (i));

      SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
          fieldReference);

      SgSubtractOp * upperBoundExpression = buildSubtractOp (
          fieldSelectionExpression, buildIntVal (1));

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              parallelLoop.get_OP_DAT_Type (i), lowerBoundExpression,
              upperBoundExpression), NULL, subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
      variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();

      appendStatement (variableDeclaration, subroutineScope);

      formalParameters->append_arg (
          *(variableDeclaration->get_variables ().begin ()));

      formalParameter_OP_DATs[i] = variableDeclaration;
    }
  }
}

KernelSubroutineOfIndirectLoop::KernelSubroutineOfIndirectLoop (
    std::string const & subroutineName,
    UserDeviceSubroutine & userDeviceSubroutine,
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
    ParallelLoop & parallelLoop, SgScopeStatement * moduleScope) :
  KernelSubroutine (subroutineName)
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionParameterList;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;
  using std::string;
  using std::vector;

  formalParameters = buildFunctionParameterList ();

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(global) ",
      AstUnparseAttribute::e_before);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createArgsSizesFormalParameter (deviceDataSizesDeclaration);

  create_OP_DAT_FormalParameters (deviceDataSizesDeclaration, parallelLoop);

  createPlanFormalParameters (deviceDataSizesDeclaration, parallelLoop);

  createLocalVariables (parallelLoop);

  createSharedLocalVariables (parallelLoop);

  createRemainingLocalVariables ();

  createThreadZeroStatements (parallelLoop);

  initialiseLocalVariables (parallelLoop);

  createAutosharedWhileLoops (parallelLoop);

  createPlanWhileLoop (userDeviceSubroutine, parallelLoop);
}
