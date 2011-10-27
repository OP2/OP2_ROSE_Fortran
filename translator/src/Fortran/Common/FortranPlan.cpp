#include <FortranPlan.h>
#include <FortranTypesBuilder.h>
#include <ParallelLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <Debug.h>
#include <Subroutine.h>

SgBasicBlock *
FortranPlan::createConvertPlanFunctionParametersStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

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
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::ind_maps, block));

  SgAggregateInitializer * parameterExpressionE3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::indsNumber));

  SgStatement
      * callStatementE =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionE1,
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::pindMaps),
              parameterExpressionE3);

  appendStatement (callStatementE, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionF1 = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan),
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::ncolblk));

  SgDotExp * parameterExpressionF3 = buildDotExp (
      variableDeclarations->getReference (OP2::VariableNames::getOpSetName ()),
      buildOpaqueVarRefExp (OP2::VariableNames::size, block));

  SgStatement
      * callStatementF =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionF1,
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::ncolblk),
              parameterExpressionF3);

  appendStatement (callStatementF, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionG = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nblocks, block));

  SgMultiplyOp * multiplyExpressionG = buildMultiplyOp (dotExpressionG,
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::indsNumber));

  SgExprStatement
      * assignmentStatementG = buildAssignStatement (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::pindSizesSize),
          multiplyExpressionG);

  appendStatement (assignmentStatementG, block);

  SgDotExp * parameterExpressionG1 = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::ind_sizes, block));

  SgAggregateInitializer * parameterExpressionG3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::pindSizesSize));

  SgStatement
      * callStatementG =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionG1,
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::pindSizes),
              parameterExpressionG3);

  appendStatement (callStatementG, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExprStatement * assignmentStatementH = buildAssignStatement (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::pindOffsSize),
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::pindSizesSize));

  appendStatement (assignmentStatementH, block);

  SgDotExp * parameterExpressionH1 = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::ind_offs, block));

  SgAggregateInitializer * parameterExpressionH3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::pindOffsSize));

  SgStatement
      * callStatementH =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionH1,
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::pindOffs),
              parameterExpressionH3);

  appendStatement (callStatementH, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionI = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nblocks, block));

  SgExprStatement * assignmentStatementI = buildAssignStatement (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::pblkMapSize), dotExpressionI);

  appendStatement (assignmentStatementI, block);

  SgDotExp * parameterExpressionI1 = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::blkmap, block));

  SgAggregateInitializer * parameterExpressionI3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::pblkMapSize));

  SgStatement
      * callStatementI =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionI1,
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::pblkMap),
              parameterExpressionI3);

  appendStatement (callStatementI, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionJ = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nblocks, block));

  SgExprStatement * assignmentStatementJ = buildAssignStatement (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::poffsetSize), dotExpressionJ);

  appendStatement (assignmentStatementJ, block);

  SgDotExp * parameterExpressionJ1 = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::offset, block));

  SgAggregateInitializer * parameterExpressionJ3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::poffsetSize));

  SgStatement
      * callStatementJ =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionJ1,
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::poffset),
              parameterExpressionJ3);

  appendStatement (callStatementJ, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionK = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nblocks, block));

  SgExprStatement * assignmentStatementK = buildAssignStatement (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::pnelemsSize), dotExpressionK);

  appendStatement (assignmentStatementK, block);

  SgDotExp * parameterExpressionK1 = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nelems, block));

  SgAggregateInitializer * parameterExpressionK3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::pnelemsSize));

  SgStatement
      * callStatementK =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionK1,
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::pnelems),
              parameterExpressionK3);

  appendStatement (callStatementK, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionL = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nblocks, block));

  SgExprStatement * assignmentStatementL = buildAssignStatement (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::pnthrcolSize), dotExpressionL);

  appendStatement (assignmentStatementL, block);

  SgDotExp * parameterExpressionL1 = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nthrcol, block));

  SgAggregateInitializer * parameterExpressionL3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::pnthrcolSize));

  SgStatement
      * callStatementL =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionL1,
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::pnthrcol),
              parameterExpressionL3);

  appendStatement (callStatementL, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExprStatement * assignmentStatementM = buildAssignStatement (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::pthrcolSize), buildDotExp (
          variableDeclarations->getReference (
              OP2::VariableNames::getOpSetName ()), buildOpaqueVarRefExp (
              OP2::VariableNames::size, block)));

  appendStatement (assignmentStatementM, block);

  SgDotExp * parameterExpressionM1 = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::thrcol, block));

  SgAggregateInitializer * parameterExpressionM3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::pthrcolSize));

  SgStatement
      * callStatementM =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionM1,
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::pthrcol),
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
            variableDeclarations->getReference (
                OP2::VariableNames::PlanFunction::pindMaps), buildIntVal (
                countIndirectArgs));

        SgPntrArrRefExp * parameterExpressionN3 = buildPntrArrRefExp (
            variableDeclarations->getReference (
                OP2::VariableNames::PlanFunction::pnindirect), buildIntVal (
                countIndirectArgs));

        SgStatement
            * callStatementN =
                FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                    subroutineScope, parameterExpressionN1,
                    variableDeclarations->getReference (
                        OP2::VariableNames::getLocalToGlobalMappingName (i)),
                    parameterExpressionN3);

        appendStatement (callStatementN, block);

        countIndirectArgs++;
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranPlan::createConvertPositionInPMapsStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

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
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::nindirect, block));

  SgAggregateInitializer * parameterExpressionB3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::indsNumber));

  SgStatement
      * callStatementB =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionB1,
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::pnindirect),
              parameterExpressionB3);

  appendStatement (callStatementB, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionC1 = buildDotExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::actualPlan), buildOpaqueVarRefExp (
          OP2::VariableNames::PlanFunction::maps, block));

  SgAggregateInitializer * parameterExpressionC3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::argsNumber));

  SgStatement
      * callStatementC =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionC1,
              variableDeclarations->getReference (
                  OP2::VariableNames::PlanFunction::pmaps),
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

      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (
              OP2::VariableNames::getOpSetName ()), buildOpaqueVarRefExp (
              OP2::VariableNames::size, block));

      SgExprStatement * statement1 = buildAssignStatement (
          variableDeclarations->getReference (
              OP2::VariableNames::getGlobalToLocalMappingSizeName (i)),
          dotExpression1);

      appendStatement (statement1, ifBody);

      SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::pmaps), buildIntVal (i));

      SgVarRefExp * parameterExpression2 = variableDeclarations->getReference (
          OP2::VariableNames::getGlobalToLocalMappingName (i));

      SgAggregateInitializer * parameterExpression3 =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->getReference (
                  OP2::VariableNames::getGlobalToLocalMappingSizeName (i)));

      SgStatement
          * statement2 =
              FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                  subroutineScope, parameterExpression1, parameterExpression2,
                  parameterExpression3);

      appendStatement (statement2, ifBody);

      /*
       * ======================================================
       * Add the if statement with a NULL else block
       * ======================================================
       */

      SgExpression * arrayIndexExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (
              OP2::VariableNames::PlanFunction::inds), buildIntVal (i));

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
FortranPlan::createPlanFunctionParametersPreparationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to prepare plan function parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        OP2::VariableNames::getOpDatName (i)), buildOpaqueVarRefExp (
        OP2::VariableNames::index, block));

    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (
            OP2::VariableNames::PlanFunction::args), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, dotExpression);

    appendStatement (assignmentStatement, block);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (
            OP2::VariableNames::PlanFunction::idxs), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, variableDeclarations->getReference (
            OP2::VariableNames::getOpIndirectionName (i)));

    appendStatement (assignmentStatement, block);
  }

  /*
   * ======================================================
   * The loop starts counting from 1
   * ======================================================
   */
  SgExpression * initializationExpression = buildAssignOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1), buildIntVal (1));

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
  SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::idxs),
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1));

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
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        OP2::VariableNames::getOpMapName (i)), buildOpaqueVarRefExp (
        OP2::VariableNames::index, block));

    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (
            OP2::VariableNames::PlanFunction::maps), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, dotExpression);

    appendStatement (assignmentStatement, block);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (
            OP2::VariableNames::PlanFunction::accesses), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, variableDeclarations->getReference (
            OP2::VariableNames::getOpAccessName (i)));

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
        variableDeclarations->getReference (
            OP2::VariableNames::PlanFunction::inds), buildIntVal (i));

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
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::argsNumber), buildIntVal (
          parallelLoop->getNumberOfOpDatArgumentGroups ()));

  appendStatement (assignmentStatement2, block);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::indsNumber), buildIntVal (
          parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()));

  appendStatement (assignmentStatement3, block);

  return block;
}

SgFunctionCallExp *
FortranPlan::createPlanFunctionCallExpression ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating plan function call statement",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVarRefExp * parameter1 = variableDeclarations->getReference (
      OP2::VariableNames::getUserSubroutineName ());

  SgDotExp * parameter2 = buildDotExp (variableDeclarations->getReference (
      OP2::VariableNames::getOpSetName ()), buildOpaqueVarRefExp (
      OP2::VariableNames::index, subroutineScope));

  SgVarRefExp * parameter3 = variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::argsNumber);

  SgVarRefExp * parameter4 = variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::args);

  SgVarRefExp * parameter5 = variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::idxs);

  SgVarRefExp * parameter6 = variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::maps);

  SgVarRefExp * parameter7 = variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::accesses);

  SgVarRefExp * parameter8 = variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::indsNumber);

  SgVarRefExp * parameter9 = variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::inds);

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

FortranPlan::FortranPlan (SgScopeStatement * subroutineScope,
    ParallelLoop * parallelLoop,
    ScopedVariableDeclarations * variableDeclarations) :
  Plan (subroutineScope, parallelLoop, variableDeclarations)
{
}

