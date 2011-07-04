#include <FortranOpenMPHostSubroutineIndirectLoop.h>
#include <CommonNamespaces.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranOpenMPHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildVarRefExp;

  FortranOpenMPModuleDeclarationsIndirectLoop
      * moduleDeclarationsIndirectLoop =
          dynamic_cast <FortranOpenMPModuleDeclarationsIndirectLoop *> (moduleDeclarations);

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OpenMP::blockID)));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      actualParameters->append_expression (buildVarRefExp (
          moduleDeclarationsIndirectLoop->getGlobalOpDatDeclaration (i)));

      actualParameters->append_expression (
          buildVarRefExp (
              moduleDeclarationsIndirectLoop->getLocalToGlobalMappingDeclaration (
                  i)));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      actualParameters->append_expression (
          buildVarRefExp (
              moduleDeclarationsIndirectLoop->getGlobalToLocalMappingDeclaration (
                  i)));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == DIRECT)
    {
      actualParameters->append_expression (buildVarRefExp (
          moduleDeclarationsIndirectLoop->getGlobalOpDatDeclaration (i)));
    }
  }

  actualParameters->append_expression (buildVarRefExp (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::Fortran::pindSizes)));

  actualParameters->append_expression (buildVarRefExp (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::Fortran::pindOffs)));

  actualParameters->append_expression (buildVarRefExp (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::Fortran::pblkMap)));

  actualParameters->append_expression (buildVarRefExp (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::Fortran::poffset)));

  actualParameters->append_expression (buildVarRefExp (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::Fortran::pnelems)));

  actualParameters->append_expression (buildVarRefExp (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::Fortran::pnthrcol)));

  actualParameters->append_expression (buildVarRefExp (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::Fortran::pthrcol)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::blockOffset)));

  SgExprStatement * callStatement =
      buildFunctionCallStmt (kernelSubroutineName, buildVoidType (),
          actualParameters, subroutineScope);

  return callStatement;
}

void
FortranOpenMPHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;
  using std::string;

  FortranOpenMPModuleDeclarationsIndirectLoop
      * moduleDeclarationsIndirectLoop =
          dynamic_cast <FortranOpenMPModuleDeclarationsIndirectLoop *> (moduleDeclarations);

  /*
   * ======================================================
   * Statement to initialise the block offset (before the
   * loops commence execution)
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::Fortran::blockOffset)), buildIntVal (0));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Build the inner loop
   * ======================================================
   */

  SgBasicBlock * innerLoopBody = buildBasicBlock ();

  /*
   * ======================================================
   * ====> Statements inside loop
   * ======================================================
   */

  appendStatement (createKernelFunctionCallStatement (), innerLoopBody);

  /*
   * ======================================================
   * ====> Lower and upper bounds
   * ======================================================
   */

  SgAssignOp * innerLoopInitializationExpression = buildAssignOp (
      buildVarRefExp (variableDeclarations->get (OpenMP::blockID)),
      buildIntVal (0));

  SgExpression * innerLoopUpperBoundExpression = buildSubtractOp (
      buildVarRefExp (
          variableDeclarations->get (PlanFunction::Fortran::nblocks)),
      buildIntVal (1));

  SgFortranDo * innerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          innerLoopInitializationExpression, innerLoopUpperBoundExpression,
          buildIntVal (1), innerLoopBody);

  /*
   * ======================================================
   * Build the outer loop
   * ======================================================
   */

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  /*
   * ======================================================
   * ====> Statements inside loop
   * ======================================================
   */

  SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::col)), buildIntVal (1));

  SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (buildVarRefExp (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::Fortran::ncolblk)), addExpression2);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (
          variableDeclarations->get (PlanFunction::Fortran::nblocks)),
      arrayIndexExpression2);

  appendStatement (assignmentStatement2, outerLoopBody);

  appendStatement (innerLoopStatement, outerLoopBody);

  SgAddOp * addExpression3 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::blockOffset)),
      buildVarRefExp (
          variableDeclarations->get (PlanFunction::Fortran::nblocks)));

  SgStatement * assignmentStatement3 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (PlanFunction::Fortran::blockOffset)),
      addExpression3);

  appendStatement (assignmentStatement3, outerLoopBody);

  /*
   * ======================================================
   * ====> Lower and upper bounds
   * ======================================================
   */

  SgAssignOp * outerLoopInitializationExpression = buildAssignOp (
      buildVarRefExp (variableDeclarations->get (CommonVariableNames::col)),
      buildIntVal (0));

  SgDotExp * dotExpression3 = buildDotExp (buildVarRefExp (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::Fortran::actualPlan)), buildOpaqueVarRefExp (
      PlanFunction::Fortran::ncolors, subroutineScope));

  SgExpression * outerLoopUpperBoundExpression = buildSubtractOp (
      dotExpression3, buildIntVal (1));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
}

void
FortranOpenMPHostSubroutineIndirectLoop::createTransferOpDatStatements (
    SgScopeStatement * statementScope)
{
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageInterface::appendStatement;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (CommonVariableNames::dim, statementScope));

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (CommonVariableNames::set, statementScope));

      SgDotExp * dotExpression3 = buildDotExp (dotExpression2,
          buildOpaqueVarRefExp (CommonVariableNames::size, statementScope));

      SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
          dotExpression3);

      FortranOpenMPModuleDeclarationsIndirectLoop
          * moduleDeclarationsIndirectLoop =
              dynamic_cast <FortranOpenMPModuleDeclarationsIndirectLoop *> (moduleDeclarations);

      SgExprStatement
          * assignmentStatement =
              buildAssignStatement (
                  buildVarRefExp (
                      moduleDeclarationsIndirectLoop->getGlobalOpDatSizeDeclaration (
                          i)), multiplyExpression);

      appendStatement (assignmentStatement, statementScope);

      SgDotExp * parameterExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (CommonVariableNames::dat, subroutineScope));

      SgVarRefExp * parameterExpression2 = buildVarRefExp (
          moduleDeclarations->getGlobalOpDatDeclaration (i));

      SgVarRefExp * parameterExpression3 = buildVarRefExp (
          moduleDeclarationsIndirectLoop->getGlobalOpDatSizeDeclaration (i));

      SgStatement * callStatement =
          SubroutineCalls::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpression1, parameterExpression2,
              buildOpaqueVarRefExp ("(/"
                  + parameterExpression3->unparseToString () + "/)",
                  subroutineScope));

      appendStatement (callStatement, statementScope);
    }
  }
}

void
FortranOpenMPHostSubroutineIndirectLoop::createFirstTimeExecutionStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildBoolValExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  using std::vector;

  vector <SubroutineVariableDeclarations *> declarationSets;
  declarationSets.push_back (variableDeclarations);
  declarationSets.push_back (
      static_cast <FortranOpenMPModuleDeclarationsIndirectLoop *> (moduleDeclarations)->getAllDeclarations ());

  SubroutineVariableDeclarations * allDeclarations =
      new SubroutineVariableDeclarations (declarationSets);

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (true));

  SgBasicBlock * ifBody = buildBasicBlock ();

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (false));

  appendStatement (assignmentStatement, ifBody);

  createPlanFunctionParametersPreparationStatements (allDeclarations,
      (FortranParallelLoop *) parallelLoop, ifBody);

  createPlanFunctionCallStatement (allDeclarations, subroutineScope, ifBody);

  createTransferOpDatStatements (ifBody);

  createConvertPositionInPMapsStatements (allDeclarations,
      (FortranParallelLoop *) parallelLoop, subroutineScope, ifBody);

  createConvertPlanFunctionParametersStatements (allDeclarations,
      (FortranParallelLoop *) parallelLoop, subroutineScope, ifBody);

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, subroutineScope);
}

void
FortranOpenMPHostSubroutineIndirectLoop::createExecutionPlanDeclarations ()
{
  using std::string;
  using std::vector;

  /*
   * ======================================================
   * Create arrays for OP_DAT, OP_INDIRECTION,
   * OP_MAP, OP_ACCESS arguments. These arrays are filled up
   * with the actual values of the OP_DAT, OP_INDIRECTION,
   * OP_MAP, OP_ACCESS that are passed to the OP_PAR_LOOP;
   * these arrays are then given to the plan function.
   * Note, therefore, that the size of the arrays is exactly
   * the number of OP_DAT argument groups.
   * There is an additional array 'inds' storing which
   * OP_DAT arguments are accessed through an indirection
   * ======================================================
   */

  vector <string> fourByteIntegerArrays;

  fourByteIntegerArrays.push_back (PlanFunction::Fortran::args);

  fourByteIntegerArrays.push_back (PlanFunction::Fortran::idxs);

  fourByteIntegerArrays.push_back (PlanFunction::Fortran::maps);

  fourByteIntegerArrays.push_back (PlanFunction::Fortran::accesses);

  fourByteIntegerArrays.push_back (PlanFunction::Fortran::inds);

  for (vector <string>::iterator it = fourByteIntegerArrays.begin (); it
      != fourByteIntegerArrays.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger (), 1,
                parallelLoop->getNumberOfOpDatArgumentGroups ()),
            subroutineScope));
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of integer fields. These fields need to be accessed
   * on the Fortran side, so create local variables that
   * enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> fourByteIntegerVariables;

  fourByteIntegerVariables.push_back (CommonVariableNames::col);

  fourByteIntegerVariables.push_back (CommonVariableNames::iterationCounter1);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::argsNumber);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::indsNumber);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::blockOffset);

  fourByteIntegerVariables.push_back (PlanFunction::Fortran::nblocks);

  for (vector <string>::iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }
}

void
FortranOpenMPHostSubroutineIndirectLoop::createOpenMPVariableDeclarations ()
{
  variableDeclarations->add (OpenMP::numberOfThreads,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::numberOfThreads, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (OpenMP::blockID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::blockID, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));
}

void
FortranOpenMPHostSubroutineIndirectLoop::createStatements ()
{
  initialiseNumberOfThreadsStatements ();

  createFirstTimeExecutionStatements ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  createPlanFunctionExecutionStatements ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
FortranOpenMPHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  createOpenMPVariableDeclarations ();

  createExecutionPlanDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionLocalVariableDeclarations ();
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranOpenMPHostSubroutineIndirectLoop::FortranOpenMPHostSubroutineIndirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations) :
  FortranOpenMPHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope, moduleDeclarations)
{
  Debug::getInstance ()->debugMessage ("<Host, Indirect, OpenMP>",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
