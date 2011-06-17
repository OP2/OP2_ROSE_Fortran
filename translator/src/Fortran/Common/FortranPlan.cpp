#include <FortranPlan.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <Debug.h>

void
FortranPlan::createPlanFunctionParametersPreparationStatements (
    ParallelLoop * parallelLoop, SgScopeStatement * scope,
    VariableDeclarations * variableDeclarations)
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
      "Creating statements to prepare plan function parameters", 5);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgVarRefExp * opDatFormalArgumentReference = buildVarRefExp (
        variableDeclarations->get (VariableNames::getOpDatName (i)));

    SgExpression * indexField = buildDotExp (opDatFormalArgumentReference,
        buildOpaqueVarRefExp ("index", scope));

    SgVarRefExp * opDatArrayReference = buildVarRefExp (
        variableDeclarations->get (
            IndirectLoop::Fortran::HostSubroutine::VariableNames::args));

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
        variableDeclarations->get (
            IndirectLoop::Fortran::HostSubroutine::VariableNames::idxs));

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
  SgExpression
      * initializationExpression =
          buildAssignOp (
              buildVarRefExp (
                  variableDeclarations->get (
                      IndirectLoop::Fortran::HostSubroutine::VariableNames::iterationCounter)),
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
  SgExpression
      * arrayIndexExpression1 =
          buildPntrArrRefExp (
              buildVarRefExp (variableDeclarations->get (
                  IndirectLoop::Fortran::HostSubroutine::VariableNames::idxs)),
              buildVarRefExp (
                  variableDeclarations->get (
                      IndirectLoop::Fortran::HostSubroutine::VariableNames::iterationCounter)));

  SgSubtractOp * subtractExpression1 = buildSubtractOp (arrayIndexExpression1,
      buildIntVal (1));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayIndexExpression1, subtractExpression1);

  SgBasicBlock * ifBody = buildBasicBlock (assignmentStatement1);

  SgExpression * ifGuardExpression = buildNotEqualOp (arrayIndexExpression1,
      buildIntVal (-1));

  SgIfStmt * ifStatement =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
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
        buildOpaqueVarRefExp ("index", scope));

    SgVarRefExp * opMapArrayReference = buildVarRefExp (
        variableDeclarations->get (
            IndirectLoop::Fortran::HostSubroutine::VariableNames::maps));

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
        variableDeclarations->get (
            IndirectLoop::Fortran::HostSubroutine::VariableNames::accesses));

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
        variableDeclarations->get (
            IndirectLoop::Fortran::HostSubroutine::VariableNames::inds)),
        buildIntVal (i));

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
          IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber)),
      buildIntVal (parallelLoop->getNumberOfOpDatArgumentGroups ()));

  appendStatement (assignmentStatement2, scope);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber)),
      buildIntVal (parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()));

  appendStatement (assignmentStatement3, scope);
}

void
FortranPlan::createPlanFunctionCallStatement (SgScopeStatement * scope,
    VariableDeclarations * variableDeclarations)
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating plan function call statement",
      5);

  SgVarRefExp * parameter1 = buildVarRefExp (variableDeclarations->get (
      VariableNames::getUserSubroutineName ()));

  SgDotExp * parameter2 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (VariableNames::getOpSetName ())),
      buildOpaqueVarRefExp ("index", scope));

  SgVarRefExp * parameter3 = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber));

  SgVarRefExp * parameter4 = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::args));

  SgVarRefExp * parameter5 = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::idxs));

  SgVarRefExp * parameter6 = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::maps));

  SgVarRefExp * parameter7 = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::accesses));

  SgVarRefExp * parameter8 = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber));

  SgVarRefExp * parameter9 = buildVarRefExp (variableDeclarations->get (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::inds));

  SgExprListExp * actualParameters = buildExprListExp (parameter1, parameter2,
      parameter3, parameter4, parameter5, parameter6, parameter7, parameter8,
      parameter9);

  SgFunctionSymbol * cplanFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("cplan", scope);

  SgFunctionCallExp * cplanFunctionCall = buildFunctionCallExp (
      cplanFunctionSymbol, actualParameters);

  SgVarRefExp * cplanFunctionReturnReference = buildVarRefExp (
      variableDeclarations->get (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet));

  SgExprStatement * assignmentStatement = buildAssignStatement (
      cplanFunctionReturnReference, cplanFunctionCall);

  appendStatement (assignmentStatement, scope);
}

void
FortranPlan::createPlanFunctionExecutionStatements (
    ParallelLoop * parallelLoop, SgScopeStatement * scope,
    VariableDeclarations * variableDeclarations)
{
}
