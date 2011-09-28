#include <FortranPlan.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <CommonNamespaces.h>
#include <Debug.h>
#include <Subroutine.h>

SgBasicBlock *
FortranPlan::createConvertPlanFunctionParametersStatements (
    FortranParallelLoop * parallelLoop, SgScopeStatement * subroutineScope,
    SubroutineVariableDeclarations * variableDeclarations)
{
  using SageBuilder::buildBasicBlock;
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

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionE1 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::ind_maps, block));

  SgExpression * parameterExpressionE3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::indsNumber), block);

  SgStatement * callStatementE =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, parameterExpressionE1, buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::PlanFunction::pindMaps)),
          parameterExpressionE3);

  appendStatement (callStatementE, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionF1 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)),
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::ncolblk)));

  SgDotExp * parameterExpressionF3 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::getOpSetName ())),
      buildOpaqueVarRefExp (OP2::VariableNames::size, block));

  SgStatement * callStatementF =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, parameterExpressionF1, buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::PlanFunction::ncolblk)),
          parameterExpressionF3);

  appendStatement (callStatementF, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionG = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nblocks, block));

  SgMultiplyOp * multiplyExpressionG = buildMultiplyOp (dotExpressionG,
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::indsNumber)));

  SgExprStatement * assignmentStatementG = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::pindSizesSize)),
      multiplyExpressionG);

  appendStatement (assignmentStatementG, block);

  SgDotExp * parameterExpressionG1 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::ind_sizes, block));

  SgExpression * parameterExpressionG3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::pindSizesSize), block);

  SgStatement * callStatementG =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, parameterExpressionG1, buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::PlanFunction::pindSizes)),
          parameterExpressionG3);

  appendStatement (callStatementG, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExprStatement * assignmentStatementH = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::pindOffsSize)), buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::pindSizesSize)));

  appendStatement (assignmentStatementH, block);

  SgDotExp * parameterExpressionH1 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::ind_offs, block));

  SgExpression * parameterExpressionH3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::pindOffsSize), block);

  SgStatement * callStatementH =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, parameterExpressionH1, buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::PlanFunction::pindOffs)),
          parameterExpressionH3);

  appendStatement (callStatementH, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionI = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nblocks, block));

  SgExprStatement * assignmentStatementI = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::pblkMapSize)), dotExpressionI);

  appendStatement (assignmentStatementI, block);

  SgDotExp * parameterExpressionI1 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::blkmap, block));

  SgExpression * parameterExpressionI3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::pblkMapSize), block);

  SgStatement * callStatementI =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, parameterExpressionI1, buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::PlanFunction::pblkMap)),
          parameterExpressionI3);

  appendStatement (callStatementI, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionJ = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nblocks, block));

  SgExprStatement * assignmentStatementJ = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::poffsetSize)), dotExpressionJ);

  appendStatement (assignmentStatementJ, block);

  SgDotExp * parameterExpressionJ1 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::offset, block));

  SgExpression * parameterExpressionJ3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::poffsetSize), block);

  SgStatement * callStatementJ =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, parameterExpressionJ1, buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::PlanFunction::poffset)),
          parameterExpressionJ3);

  appendStatement (callStatementJ, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionK = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nblocks, block));

  SgExprStatement * assignmentStatementK = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::pnelemsSize)), dotExpressionK);

  appendStatement (assignmentStatementK, block);

  SgDotExp * parameterExpressionK1 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nelems, block));

  SgExpression * parameterExpressionK3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::pnelemsSize), block);

  SgStatement * callStatementK =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, parameterExpressionK1, buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::PlanFunction::pnelems)),
          parameterExpressionK3);

  appendStatement (callStatementK, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionL = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nblocks, block));

  SgExprStatement * assignmentStatementL = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::pnthrcolSize)), dotExpressionL);

  appendStatement (assignmentStatementL, block);

  SgDotExp * parameterExpressionL1 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nthrcol, block));

  SgExpression * parameterExpressionL3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::pnthrcolSize), block);

  SgStatement * callStatementL =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, parameterExpressionL1, buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::PlanFunction::pnthrcol)),
          parameterExpressionL3);

  appendStatement (callStatementL, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExprStatement * assignmentStatementM = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::pthrcolSize)), buildDotExp (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpSetName ())), buildOpaqueVarRefExp (
              OP2::VariableNames::size, block)));

  appendStatement (assignmentStatementM, block);

  SgDotExp * parameterExpressionM1 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::thrcol, block));

  SgExpression * parameterExpressionM3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::pthrcolSize), block);

  SgStatement * callStatementM =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, parameterExpressionM1, buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::PlanFunction::pthrcol)),
          parameterExpressionM3);

  appendStatement (callStatementM, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion calls
   * ======================================================
   */

  unsigned int countIndirectArgs = 1;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgPntrArrRefExp * parameterExpressionN1 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::PlanFunction::pindMaps)), buildIntVal (
                countIndirectArgs));

        SgPntrArrRefExp * parameterExpressionN3 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::PlanFunction::pnindirect)), buildIntVal (
                countIndirectArgs));

        SgStatement * callStatementN =
            SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
                subroutineScope, parameterExpressionN1, buildVarRefExp (
                    variableDeclarations->get (
                        OP2::VariableNames::getLocalToGlobalMappingName (i))),
                parameterExpressionN3);

        appendStatement (callStatementN, block);

        countIndirectArgs++;
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranPlan::createConvertPositionInPMapsStatements (
    FortranParallelLoop * parallelLoop, SgScopeStatement * subroutineScope,
    SubroutineVariableDeclarations * variableDeclarations)
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

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionB1 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nindirect, block));

  SgExpression * parameterExpressionB3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::indsNumber), block);

  SgStatement * callStatementB =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, parameterExpressionB1, buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::PlanFunction::pnindirect)),
          parameterExpressionB3);

  appendStatement (callStatementB, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionC1 = buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::maps, block));

  SgExpression * parameterExpressionC3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::argsNumber), block);

  SgStatement * callStatementC =
      SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
          subroutineScope, parameterExpressionC1, buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::PlanFunction::pmaps)),
          parameterExpressionC3);

  appendStatement (callStatementC, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion calls
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      /*
       * ======================================================
       * Build the body of the if statement
       * ======================================================
       */

      SgBasicBlock * ifBody = buildBasicBlock ();

      SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpSetName ())),
          buildOpaqueVarRefExp (OP2::VariableNames::size, block));

      SgExprStatement * statement1 = buildAssignStatement (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getGlobalToLocalMappingSizeName (i))),
          dotExpression1);

      appendStatement (statement1, ifBody);

      SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::PlanFunction::pmaps)), buildIntVal (i));

      SgVarRefExp * parameterExpression2 = buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getGlobalToLocalMappingName (i)));

      SgExpression * parameterExpression3 =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (
                  OP2::VariableNames::getGlobalToLocalMappingSizeName (i)),
              block);

      SgStatement * statement2 =
          SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpression1, parameterExpression2,
              parameterExpression3);

      appendStatement (statement2, ifBody);

      /*
       * ======================================================
       * Add the if statement with a NULL else block
       * ======================================================
       */

      SgExpression * arrayIndexExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::PlanFunction::inds)),
          buildIntVal (i));

      SgExpression * ifGuardExpression = buildGreaterOrEqualOp (
          arrayIndexExpression, buildIntVal (0));

      SgIfStmt * ifStatement =
          RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
              ifGuardExpression, ifBody);

      appendStatement (ifStatement, block);
    }
  }

  return block;
}

SgBasicBlock *
FortranPlan::createPlanFunctionParametersPreparationStatements (
    FortranParallelLoop * parallelLoop,
    SubroutineVariableDeclarations * variableDeclarations)
{
  using SageBuilder::buildBasicBlock;
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

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
        variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
        buildOpaqueVarRefExp (OP2::VariableNames::index, block));

    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::PlanFunction::args)), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, dotExpression);

    appendStatement (assignmentStatement, block);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * arrayIndexExpression = buildPntrArrRefExp (buildVarRefExp (
        variableDeclarations->get (OP2::VariableNames::PlanFunction::idxs)),
        buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::getOpIndirectionName (i))));

    appendStatement (assignmentStatement, block);
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
   * Build the body of the do-loop
   * ======================================================
   */
  SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::idxs)),
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)));

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
          initializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  appendStatement (fortranDoStatement, block);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
        variableDeclarations->get (OP2::VariableNames::getOpMapName (i))),
        buildOpaqueVarRefExp (OP2::VariableNames::index, block));

    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::PlanFunction::maps)), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, dotExpression);

    appendStatement (assignmentStatement, block);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::PlanFunction::accesses)), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::getOpAccessName (i))));

    appendStatement (assignmentStatement, block);
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
    if (parallelLoop->isIndirect (i))
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
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::PlanFunction::inds)), buildIntVal (i));

    SgExprStatement * assignmentStatement;

    if (parallelLoop->isIndirect (i))
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

    appendStatement (assignmentStatement, block);
  }

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::argsNumber)), buildIntVal (
          parallelLoop->getNumberOfOpDatArgumentGroups ()));

  appendStatement (assignmentStatement2, block);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::indsNumber)), buildIntVal (
          parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()));

  appendStatement (assignmentStatement3, block);

  return block;
}

SgFunctionCallExp *
FortranPlan::createPlanFunctionCallExpression (
    SgScopeStatement * subroutineScope,
    SubroutineVariableDeclarations * variableDeclarations)
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating plan function call statement",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVarRefExp * parameter1 = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::getUserSubroutineName ()));

  SgDotExp * parameter2 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::getOpSetName ())),
      buildOpaqueVarRefExp (OP2::VariableNames::index, subroutineScope));

  SgVarRefExp * parameter3 = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::argsNumber));

  SgVarRefExp * parameter4 = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::args));

  SgVarRefExp * parameter5 = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::idxs));

  SgVarRefExp * parameter6 = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::maps));

  SgVarRefExp * parameter7 = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::accesses));

  SgVarRefExp * parameter8 = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::indsNumber));

  SgVarRefExp * parameter9 = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::inds));

  SgExprListExp * actualParameters = buildExprListExp (parameter1, parameter2,
      parameter3, parameter4, parameter5, parameter6, parameter7, parameter8,
      parameter9);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction (
          OP2::VariableNames::PlanFunction::cplan, subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  return functionCall;
}
