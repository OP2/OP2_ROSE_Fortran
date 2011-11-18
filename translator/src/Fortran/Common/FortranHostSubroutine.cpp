#include "FortranHostSubroutine.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "FortranParallelLoop.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "OP2Definitions.h"
#include "PlanFunctionNames.h"
#include "CompilerGeneratedNames.h"
#include "Debug.h"
#include <boost/algorithm/string/predicate.hpp>
#include <rose.h>

SgBasicBlock *
FortranHostSubroutine::createConvertPlanFunctionParametersStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;

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
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          ind_maps, block));

  SgAggregateInitializer * parameterExpressionE3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (indsNumber));

  SgStatement
      * callStatementE =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionE1,
              variableDeclarations->getReference (pindMaps),
              parameterExpressionE3);

  appendStatement (callStatementE, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionF1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())),
      variableDeclarations->getReference (ncolblk));

  SgDotExp * parameterExpressionF3 = buildDotExp (
      variableDeclarations->getReference (set), buildOpaqueVarRefExp (size,
          block));

  SgStatement
      * callStatementF =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionF1,
              variableDeclarations->getReference (ncolblk),
              parameterExpressionF3);

  appendStatement (callStatementF, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionG = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nblocks, block));

  SgMultiplyOp * multiplyExpressionG = buildMultiplyOp (dotExpressionG,
      variableDeclarations->getReference (indsNumber));

  SgExprStatement * assignmentStatementG = buildAssignStatement (
      variableDeclarations->getReference (pindSizesSize), multiplyExpressionG);

  appendStatement (assignmentStatementG, block);

  SgDotExp * parameterExpressionG1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          ind_sizes, block));

  SgAggregateInitializer * parameterExpressionG3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pindSizesSize));

  SgStatement
      * callStatementG =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionG1,
              variableDeclarations->getReference (pindSizes),
              parameterExpressionG3);

  appendStatement (callStatementG, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExprStatement * assignmentStatementH = buildAssignStatement (
      variableDeclarations->getReference (pindOffsSize),
      variableDeclarations->getReference (pindSizesSize));

  appendStatement (assignmentStatementH, block);

  SgDotExp * parameterExpressionH1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          ind_offs, block));

  SgAggregateInitializer * parameterExpressionH3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pindOffsSize));

  SgStatement
      * callStatementH =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionH1,
              variableDeclarations->getReference (pindOffs),
              parameterExpressionH3);

  appendStatement (callStatementH, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionI = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nblocks, block));

  SgExprStatement * assignmentStatementI = buildAssignStatement (
      variableDeclarations->getReference (pblkMapSize), dotExpressionI);

  appendStatement (assignmentStatementI, block);

  SgDotExp * parameterExpressionI1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          blkmap, block));

  SgAggregateInitializer * parameterExpressionI3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pblkMapSize));

  SgStatement
      * callStatementI =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionI1,
              variableDeclarations->getReference (pblkMap),
              parameterExpressionI3);

  appendStatement (callStatementI, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionJ = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nblocks, block));

  SgExprStatement * assignmentStatementJ = buildAssignStatement (
      variableDeclarations->getReference (poffsetSize), dotExpressionJ);

  appendStatement (assignmentStatementJ, block);

  SgDotExp * parameterExpressionJ1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          offset, block));

  SgAggregateInitializer * parameterExpressionJ3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (poffsetSize));

  SgStatement
      * callStatementJ =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionJ1,
              variableDeclarations->getReference (poffset),
              parameterExpressionJ3);

  appendStatement (callStatementJ, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionK = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nblocks, block));

  SgExprStatement * assignmentStatementK = buildAssignStatement (
      variableDeclarations->getReference (pnelemsSize), dotExpressionK);

  appendStatement (assignmentStatementK, block);

  SgDotExp * parameterExpressionK1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nelems, block));

  SgAggregateInitializer * parameterExpressionK3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pnelemsSize));

  SgStatement
      * callStatementK =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionK1,
              variableDeclarations->getReference (pnelems),
              parameterExpressionK3);

  appendStatement (callStatementK, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExpression * dotExpressionL = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nblocks, block));

  SgExprStatement * assignmentStatementL = buildAssignStatement (
      variableDeclarations->getReference (pnthrcolSize), dotExpressionL);

  appendStatement (assignmentStatementL, block);

  SgDotExp * parameterExpressionL1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nthrcol, block));

  SgAggregateInitializer * parameterExpressionL3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pnthrcolSize));

  SgStatement
      * callStatementL =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionL1,
              variableDeclarations->getReference (pnthrcol),
              parameterExpressionL3);

  appendStatement (callStatementL, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgExprStatement * assignmentStatementM = buildAssignStatement (
      variableDeclarations->getReference (pthrcolSize), buildDotExp (
          variableDeclarations->getReference (set), buildOpaqueVarRefExp (size,
              block)));

  appendStatement (assignmentStatementM, block);

  SgDotExp * parameterExpressionM1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          thrcol, block));

  SgAggregateInitializer * parameterExpressionM3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (pthrcolSize));

  SgStatement
      * callStatementM =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionM1,
              variableDeclarations->getReference (pthrcol),
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
            variableDeclarations->getReference (pindMaps), buildIntVal (
                countIndirectArgs));

        SgPntrArrRefExp * parameterExpressionN3 = buildPntrArrRefExp (
            variableDeclarations->getReference (pnindirect), buildIntVal (
                countIndirectArgs));

        SgStatement
            * callStatementN =
                FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                    subroutineScope, parameterExpressionN1,
                    variableDeclarations->getReference (
                        getLocalToGlobalMappingName (i)), parameterExpressionN3);

        appendStatement (callStatementN, block);

        countIndirectArgs++;
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranHostSubroutine::createConvertPositionInPMapsStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace OP2VariableNames;

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
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nindirect, block));

  SgAggregateInitializer * parameterExpressionB3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (indsNumber));

  SgStatement
      * callStatementB =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionB1,
              variableDeclarations->getReference (pnindirect),
              parameterExpressionB3);

  appendStatement (callStatementB, block);

  /*
   * ======================================================
   * New C-To-Fortran pointer conversion call
   * ======================================================
   */

  SgDotExp * parameterExpressionC1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (maps,
          block));

  SgAggregateInitializer * parameterExpressionC3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (argsNumber));

  SgStatement
      * callStatementC =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionC1,
              variableDeclarations->getReference (pmaps), parameterExpressionC3);

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
          variableDeclarations->getReference (set), buildOpaqueVarRefExp (size,
              block));

      SgExprStatement * statement1 = buildAssignStatement (
          variableDeclarations->getReference (getGlobalToLocalMappingSizeName (
              i)), dotExpression1);

      appendStatement (statement1, ifBody);

      SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (pmaps), buildIntVal (i));

      SgVarRefExp * parameterExpression2 = variableDeclarations->getReference (
          getGlobalToLocalMappingName (i));

      SgAggregateInitializer * parameterExpression3 =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->getReference (
                  getGlobalToLocalMappingSizeName (i)));

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
          variableDeclarations->getReference (inds), buildIntVal (i));

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
FortranHostSubroutine::createPlanFunctionParametersPreparationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to prepare plan function parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpDatName (i)), buildOpaqueVarRefExp (
        OP2::RunTimeVariableNames::index, block));

    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (args), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, dotExpression);

    appendStatement (assignmentStatement, block);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (idxs), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, variableDeclarations->getReference (
            getOpIndirectionName (i)));

    appendStatement (assignmentStatement, block);
  }

  /*
   * ======================================================
   * The loop starts counting from 1
   * ======================================================
   */
  SgExpression * initializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
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
  SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (idxs),
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

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
        getOpMapName (i)), buildOpaqueVarRefExp (
        OP2::RunTimeVariableNames::index, block));

    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (maps), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, dotExpression);

    appendStatement (assignmentStatement, block);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (accesses), buildIntVal (i));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, variableDeclarations->getReference (
            getOpAccessName (i)));

    appendStatement (assignmentStatement, block);
  }

  /*
   * ======================================================
   * Set up a mapping between OP_DATs and indirection
   * values. At the beginning everything is set to undefined
   * ======================================================
   */
  int const undefinedIndex = -2;

  std::map <std::string, int> indexValues;

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
        variableDeclarations->getReference (inds), buildIntVal (i));

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
      variableDeclarations->getReference (argsNumber), buildIntVal (
          parallelLoop->getNumberOfOpDatArgumentGroups ()));

  appendStatement (assignmentStatement2, block);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (indsNumber), buildIntVal (
          parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()));

  appendStatement (assignmentStatement3, block);

  return block;
}

SgFunctionCallExp *
FortranHostSubroutine::createPlanFunctionCallExpression ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage ("Creating plan function call statement",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVarRefExp * parameter1 = variableDeclarations->getReference (
      getUserSubroutineName ());

  SgDotExp * parameter2 = buildDotExp (variableDeclarations->getReference (
      getOpSetName ()), buildOpaqueVarRefExp (OP2::RunTimeVariableNames::index,
      subroutineScope));

  SgVarRefExp * parameter3 = variableDeclarations->getReference (argsNumber);

  SgVarRefExp * parameter4 = variableDeclarations->getReference (args);

  SgVarRefExp * parameter5 = variableDeclarations->getReference (idxs);

  SgVarRefExp * parameter6 = variableDeclarations->getReference (maps);

  SgVarRefExp * parameter7 = variableDeclarations->getReference (accesses);

  SgVarRefExp * parameter8 = variableDeclarations->getReference (indsNumber);

  SgVarRefExp * parameter9 = variableDeclarations->getReference (inds);

  SgExprListExp * actualParameters = buildExprListExp (parameter1, parameter2,
      parameter3, parameter4, parameter5, parameter6, parameter7, parameter8,
      parameter9);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction (cplan, subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  return functionCall;
}

void
FortranHostSubroutine::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2;
  using namespace OP2VariableNames;
  using boost::iequals;
  using std::vector;
  using std::string;
  using std::map;

  Debug::getInstance ()->debugMessage (
      "Creating host subroutine formal parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Add name of user kernel
   * ======================================================
   */

  string const & kernelVariableName = getUserSubroutineName ();

  variableDeclarations->add (
      kernelVariableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          kernelVariableName, FortranTypesBuilder::getString (
              parallelLoop->getUserSubroutineName ().length ()),
          subroutineScope, formalParameters, 1, INTENT_IN));

  /*
   * ======================================================
   * Add OP_SET
   * ======================================================
   */

  string const & opSetVariableName = getOpSetName ();

  variableDeclarations->add (
      opSetVariableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opSetVariableName, FortranTypesBuilder::buildClassDeclaration (
              OP_SET, subroutineScope)->get_type (), subroutineScope,
          formalParameters, 1, INTENT_IN));

  /*
   * ======================================================
   * Add OP_DAT, indirection, OP_MAP, access arguments for
   * each OP_DAT argument group
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & opDatvariableName = getOpDatName (i);

    variableDeclarations->add (
        opDatvariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opDatvariableName, FortranTypesBuilder::buildClassDeclaration (
                OP2::OP_DAT, subroutineScope)->get_type (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & indirectionVariableName = getOpIndirectionName (i);

    variableDeclarations->add (
        indirectionVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            indirectionVariableName,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & opMapVariableName = getOpMapName (i);

    variableDeclarations->add (
        opMapVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opMapVariableName, FortranTypesBuilder::buildClassDeclaration (
                OP2::OP_MAP, subroutineScope)->get_type (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & accessVariableName = getOpAccessName (i);

    variableDeclarations->add (
        accessVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            accessVariableName, FortranTypesBuilder::getFourByteInteger (),
            subroutineScope, formalParameters, 1, INTENT_IN));
  }
}

FortranHostSubroutine::FortranHostSubroutine (SgScopeStatement * moduleScope,
    Subroutine <SgProcedureHeaderStatement> * calleeSubroutine,
    FortranParallelLoop * parallelLoop) :
  HostSubroutine <SgProcedureHeaderStatement> (calleeSubroutine, parallelLoop)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  appendStatement (
      FortranStatementsAndExpressionsBuilder::buildImplicitNoneStatement (),
      subroutineScope);
}
