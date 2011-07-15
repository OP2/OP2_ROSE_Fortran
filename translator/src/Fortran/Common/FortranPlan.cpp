#include <FortranPlan.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <CommonNamespaces.h>
#include <Debug.h>

void
FortranPlan::createConvertPlanFunctionParametersStatements (
    SubroutineVariableDeclarations * variableDeclarations,
    FortranParallelLoop * parallelLoop, SgScopeStatement * subroutineScope,
    SgScopeStatement * statementScope)
{
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating statements to convert plan function parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionE1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::ind_maps, statementScope));

  SgVarRefExp * parameterExpressionE2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pindMaps));

  SgExpression * parameterExpressionE3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (PlanFunction::Fortran::indsNumber),
          statementScope);

  SgStatement * callStatementE =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionE1, parameterExpressionE2, parameterExpressionE3);

  appendStatement (callStatementE, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionF1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildVarRefExp (
          variableDeclarations->get (PlanFunction::Fortran::ncolblk)));

  SgVarRefExp * parameterExpressionF2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::ncolblk));

  SgDotExp * parameterExpressionF3 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (VariableNames::getOpSetName ())),
      buildOpaqueVarRefExp (CommonVariableNames::size, statementScope));

  SgStatement * callStatementF =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionF1, parameterExpressionF2, parameterExpressionF3);

  appendStatement (callStatementF, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionG = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::nblocks, statementScope));

  SgMultiplyOp * multiplyExpressionG = buildMultiplyOp (dotExpressionG,
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::indsNumber)));

  SgExprStatement * assignmentStatementG = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::pindSizesSize)), multiplyExpressionG);

  appendStatement (assignmentStatementG, statementScope);

  SgDotExp * parameterExpressionG1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::ind_sizes, statementScope));

  SgVarRefExp * parameterExpressionG2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pindSizes));

  SgExpression * parameterExpressionG3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (PlanFunction::Fortran::pindSizesSize),
          statementScope);

  SgStatement * callStatementG =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionG1, parameterExpressionG2, parameterExpressionG3);

  appendStatement (callStatementG, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExprStatement * assignmentStatementH = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::pindOffsSize)), buildVarRefExp (
          variableDeclarations->get (PlanFunction::Fortran::pindSizesSize)));

  appendStatement (assignmentStatementH, statementScope);

  SgDotExp * parameterExpressionH1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::ind_offs, statementScope));

  SgVarRefExp * parameterExpressionH2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pindOffs));

  SgExpression * parameterExpressionH3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (PlanFunction::Fortran::pindOffsSize),
          statementScope);

  SgStatement * callStatementH =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionH1, parameterExpressionH2, parameterExpressionH3);

  appendStatement (callStatementH, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionI = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::nblocks, statementScope));

  SgExprStatement * assignmentStatementI = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::pblkMapSize)), dotExpressionI);

  appendStatement (assignmentStatementI, statementScope);

  SgDotExp * parameterExpressionI1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::blkmap, statementScope));

  SgVarRefExp * parameterExpressionI2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pblkMap));

  SgExpression * parameterExpressionI3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (PlanFunction::Fortran::pblkMapSize),
          statementScope);

  SgStatement * callStatementI =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionI1, parameterExpressionI2, parameterExpressionI3);

  appendStatement (callStatementI, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionJ = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::nblocks, statementScope));

  SgExprStatement * assignmentStatementJ = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::poffsetSize)), dotExpressionJ);

  appendStatement (assignmentStatementJ, statementScope);

  SgDotExp * parameterExpressionJ1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::offset, statementScope));

  SgVarRefExp * parameterExpressionJ2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::poffset));

  SgExpression * parameterExpressionJ3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (PlanFunction::Fortran::poffsetSize),
          statementScope);

  SgStatement * callStatementJ =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionJ1, parameterExpressionJ2, parameterExpressionJ3);

  appendStatement (callStatementJ, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionK = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::nblocks, statementScope));

  SgExprStatement * assignmentStatementK = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::pnelemsSize)), dotExpressionK);

  appendStatement (assignmentStatementK, statementScope);

  SgDotExp * parameterExpressionK1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::nelems, statementScope));

  SgVarRefExp * parameterExpressionK2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pnelems));

  SgExpression * parameterExpressionK3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (PlanFunction::Fortran::pnelemsSize),
          statementScope);

  SgStatement * callStatementK =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionK1, parameterExpressionK2, parameterExpressionK3);

  appendStatement (callStatementK, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionL = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::nblocks, statementScope));

  SgExprStatement * assignmentStatementL = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::pthrcolSize)), dotExpressionL);

  appendStatement (assignmentStatementL, statementScope);

  SgDotExp * parameterExpressionL1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::nthrcol, statementScope));

  SgVarRefExp * parameterExpressionL2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pnthrcol));

  SgExpression * parameterExpressionL3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (PlanFunction::Fortran::pnthrcolSize),
          statementScope);

  SgStatement * callStatementL =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionL1, parameterExpressionL2, parameterExpressionL3);

  appendStatement (callStatementL, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExprStatement * assignmentStatementM = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::pthrcolSize)), buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpSetName ())),
          buildOpaqueVarRefExp (CommonVariableNames::size, statementScope)));

  appendStatement (assignmentStatementM, statementScope);

  SgDotExp * parameterExpressionM1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::thrcol, statementScope));

  SgVarRefExp * parameterExpressionM2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pthrcol));

  SgExpression * parameterExpressionM3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (PlanFunction::Fortran::pthrcolSize),
          statementScope);

  SgStatement * callStatementM =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionM1, parameterExpressionM2, parameterExpressionM3);

  appendStatement (callStatementM, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion calls
   * ======================================================
   */

  unsigned int countIndirectArgs = 1;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgPntrArrRefExp * parameterExpressionN1 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              PlanFunction::Fortran::pindMaps)),
          buildIntVal (countIndirectArgs));

      SgVarRefExp * parameterExpressionN2 = buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getLocalToGlobalMappingName (i)));

      SgPntrArrRefExp * parameterExpressionN3 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              PlanFunction::Fortran::pnindirect)), buildIntVal (
              countIndirectArgs));

      SgStatement * callStatementN =
          SubroutineCalls::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionN1, parameterExpressionN2,
              parameterExpressionN3);

      appendStatement (callStatementN, statementScope);

      countIndirectArgs++;
    }
  }
}

void
FortranPlan::createConvertPositionInPMapsStatements (
    SubroutineVariableDeclarations * variableDeclarations,
    FortranParallelLoop * parallelLoop, SgScopeStatement * subroutineScope,
    SgScopeStatement * statementScope)
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildGreaterOrEqualOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildDotExp;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating statements in Fortran plan to convert positions in PMaps",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgVarRefExp * parameterExpressionA1 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::planRet));

  SgVarRefExp * parameterExpressionA2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan));

  SgStatement * callStatementA =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionA1, parameterExpressionA2);

  appendStatement (callStatementA, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionB1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::nindirect, statementScope));

  SgVarRefExp * parameterExpressionB2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pnindirect));

  SgExpression * parameterExpressionB3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (PlanFunction::Fortran::indsNumber),
          statementScope);

  SgStatement * callStatementB =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionB1, parameterExpressionB2, parameterExpressionB3);

  appendStatement (callStatementB, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionC1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::actualPlan)),
      buildOpaqueVarRefExp (PlanFunction::Fortran::maps, statementScope));

  SgVarRefExp * parameterExpressionC2 = buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::pmaps));

  SgExpression * parameterExpressionC3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (PlanFunction::Fortran::argsNumber),
          statementScope);

  SgStatement * callStatementC =
      SubroutineCalls::createCToFortranPointerCallStatement (subroutineScope,
          parameterExpressionC1, parameterExpressionC2, parameterExpressionC3);

  appendStatement (callStatementC, statementScope);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion calls
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      /*
       * ======================================================
       * Build the body of the if statement
       * ======================================================
       */

      SgBasicBlock * ifBody = buildBasicBlock ();

      SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpSetName ())),
          buildOpaqueVarRefExp (CommonVariableNames::size, statementScope));

      SgExprStatement * statement1 = buildAssignStatement (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getGlobalToLocalMappingSizeName (i))),
          dotExpression1);

      appendStatement (statement1, ifBody);

      SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              PlanFunction::Fortran::pmaps)), buildIntVal (i));

      SgVarRefExp * parameterExpression2 = buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getGlobalToLocalMappingName (i)));

      SgExpression * parameterExpression3 =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  VariableNames::getGlobalToLocalMappingSizeName (i)),
              statementScope);

      SgStatement * statement2 =
          SubroutineCalls::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpression1, parameterExpression2,
              parameterExpression3);

      appendStatement (statement2, ifBody);

      /*
       * ======================================================
       * Add the if statement with a NULL else block
       * ======================================================
       */

      SgExpression * arrayIndexExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (PlanFunction::Fortran::inds)),
          buildIntVal (i));

      SgExpression * ifGuardExpression = buildGreaterOrEqualOp (
          arrayIndexExpression, buildIntVal (0));

      SgIfStmt * ifStatement =
          RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
              ifGuardExpression, ifBody);

      appendStatement (ifStatement, statementScope);
    }
  }
}

void
FortranPlan::createPlanFunctionParametersPreparationStatements (
    SubroutineVariableDeclarations * variableDeclarations,
    FortranParallelLoop * parallelLoop, SgScopeStatement * scope)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildNotEqualOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to prepare plan function parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgVarRefExp * opDatFormalArgumentReference = buildVarRefExp (
        variableDeclarations->get (VariableNames::getOpDatName (i)));

    SgExpression * indexField = buildDotExp (opDatFormalArgumentReference,
        buildOpaqueVarRefExp (CommonVariableNames::index, scope));

    SgVarRefExp * opDatArrayReference = buildVarRefExp (
        variableDeclarations->get (PlanFunction::Fortran::args));

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opDatArrayReference, indexExpression);

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, indexField);

    appendStatement (assignmentStatement, scope);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgVarRefExp * opIndexFormalArgumentReference = buildVarRefExp (
        variableDeclarations->get (VariableNames::getOpIndirectionName (i)));

    SgVarRefExp * opIndirectionArrayReference = buildVarRefExp (
        variableDeclarations->get (PlanFunction::Fortran::idxs));

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opIndirectionArrayReference, indexExpression);

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, opIndexFormalArgumentReference);

    appendStatement (assignmentStatement, scope);
  }

  /*
   * ======================================================
   * The loop starts counting from 1
   * ======================================================
   */
  SgExpression * initializationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildIntVal (1));

  /*
   * ======================================================
   * The loop stops counting at the number of OP_DAT argument
   * groups
   * ======================================================
   */
  SgExpression * upperBoundExpression = buildIntVal (
      parallelLoop->getNumberOfOpDatArgumentGroups ());

  /*
   * ======================================================
   * The stride of the loop counter is 1
   * ======================================================
   */
  SgExpression * strideExpression = buildIntVal (1);

  /*
   * ======================================================
   * Build the body of the do-loop
   * ======================================================
   */
  SgExpression * arrayIndexExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::idxs)), buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)));

  SgSubtractOp * subtractExpression1 = buildSubtractOp (arrayIndexExpression1,
      buildIntVal (1));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayIndexExpression1, subtractExpression1);

  SgBasicBlock * ifBody = buildBasicBlock (assignmentStatement1);

  SgExpression * ifGuardExpression = buildNotEqualOp (arrayIndexExpression1,
      buildIntVal (-1));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  SgBasicBlock * loopBody = buildBasicBlock (ifStatement);

  SgFortranDo * fortranDoStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, strideExpression,
          loopBody);

  appendStatement (fortranDoStatement, scope);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgVarRefExp * opMapFormalArgumentReference = buildVarRefExp (
        variableDeclarations->get (VariableNames::getOpMapName (i)));

    SgExpression * indexField = buildDotExp (opMapFormalArgumentReference,
        buildOpaqueVarRefExp (CommonVariableNames::index, scope));

    SgVarRefExp * opMapArrayReference = buildVarRefExp (
        variableDeclarations->get (PlanFunction::Fortran::maps));

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opMapArrayReference, indexExpression);

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, indexField);

    appendStatement (assignmentStatement, scope);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgVarRefExp * opAccessFormalArgumentReference = buildVarRefExp (
        variableDeclarations->get (VariableNames::getOpAccessName (i)));

    SgVarRefExp * opAccessArrayReference = buildVarRefExp (
        variableDeclarations->get (PlanFunction::Fortran::accesses));

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opAccessArrayReference, indexExpression);

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, opAccessFormalArgumentReference);

    appendStatement (assignmentStatement, scope);
  }

  /*
   * ======================================================
   * Set up a mapping between OP_DATs and indirection
   * values. At the beginning everything is set to undefined
   * ======================================================
   */
  int const undefinedIndex = -2;

  map <string, int> indexValues;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      indexValues[parallelLoop->getOpDatVariableName (i)] = undefinedIndex;
    }
  }

  /*
   * ======================================================
   * Start at the value defined by Mike Giles in his
   * implementation
   * ======================================================
   */
  unsigned int nextIndex = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * arrayIndexExpression = buildPntrArrRefExp (buildVarRefExp (
        variableDeclarations->get (PlanFunction::Fortran::inds)), buildIntVal (
        i));

    SgExprStatement * assignmentStatement;

    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      if (indexValues[parallelLoop->getOpDatVariableName (i)] == undefinedIndex)
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
            buildIntVal (nextIndex));

        indexValues[parallelLoop->getOpDatVariableName (i)] = nextIndex;

        nextIndex++;
      }
      else
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
            buildIntVal (indexValues[parallelLoop->getOpDatVariableName (i)]));
      }
    }
    else
    {
      assignmentStatement = buildAssignStatement (arrayIndexExpression,
          buildIntVal (-1));
    }

    appendStatement (assignmentStatement, scope);
  }

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::argsNumber)), buildIntVal (
          parallelLoop->getNumberOfOpDatArgumentGroups ()));

  appendStatement (assignmentStatement2, scope);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::indsNumber)), buildIntVal (
          parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()));

  appendStatement (assignmentStatement3, scope);
}

void
FortranPlan::createPlanFunctionCallStatement (
    SubroutineVariableDeclarations * variableDeclarations,
    SgScopeStatement * subroutineScope, SgScopeStatement * statementScope)
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating plan function call statement",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVarRefExp * parameter1 = buildVarRefExp (variableDeclarations->get (
      VariableNames::getUserSubroutineName ()));

  SgDotExp * parameter2 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (VariableNames::getOpSetName ())),
      buildOpaqueVarRefExp (CommonVariableNames::index, statementScope));

  SgVarRefExp * parameter3 = buildVarRefExp (variableDeclarations->get (
      PlanFunction::Fortran::argsNumber));

  SgVarRefExp * parameter4 = buildVarRefExp (variableDeclarations->get (
      PlanFunction::Fortran::args));

  SgVarRefExp * parameter5 = buildVarRefExp (variableDeclarations->get (
      PlanFunction::Fortran::idxs));

  SgVarRefExp * parameter6 = buildVarRefExp (variableDeclarations->get (
      PlanFunction::Fortran::maps));

  SgVarRefExp * parameter7 = buildVarRefExp (variableDeclarations->get (
      PlanFunction::Fortran::accesses));

  SgVarRefExp * parameter8 = buildVarRefExp (variableDeclarations->get (
      PlanFunction::Fortran::indsNumber));

  SgVarRefExp * parameter9 = buildVarRefExp (variableDeclarations->get (
      PlanFunction::Fortran::inds));

  SgExprListExp * actualParameters = buildExprListExp (parameter1, parameter2,
      parameter3, parameter4, parameter5, parameter6, parameter7, parameter8,
      parameter9);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction (
          PlanFunction::Fortran::cplan, subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  SgExprStatement * assignmentStatement =
      buildAssignStatement (buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::planRet)), functionCall);

  appendStatement (assignmentStatement, statementScope);
}
