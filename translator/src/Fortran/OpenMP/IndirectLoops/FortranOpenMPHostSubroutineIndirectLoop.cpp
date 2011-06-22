#include <FortranOpenMPHostSubroutineIndirectLoop.h>
#include <CommonNamespaces.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <Debug.h>

SgStatement *
FortranOpenMPHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
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
          buildOpaqueVarRefExp (
              IndirectAndDirectLoop::Fortran::HostSubroutine::dim,
              statementScope));

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (
              IndirectAndDirectLoop::Fortran::HostSubroutine::set,
              statementScope));

      SgDotExp * dotExpression3 = buildDotExp (dotExpression2,
          buildOpaqueVarRefExp (
              IndirectAndDirectLoop::Fortran::HostSubroutine::size,
              statementScope));

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
          buildOpaqueVarRefExp (
              IndirectAndDirectLoop::Fortran::HostSubroutine::dat,
              subroutineScope));

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

  vector <VariableDeclarations *> declarationSets;
  declarationSets.push_back (variableDeclarations);
  declarationSets.push_back (
      dynamic_cast <FortranOpenMPModuleDeclarationsIndirectLoop *> (moduleDeclarations)->getAllDeclarations ());

  VariableDeclarations * allDeclarations = new VariableDeclarations (
      declarationSets);

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (true));

  SgBasicBlock * ifBody = buildBasicBlock ();

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (false));

  appendStatement (assignmentStatement, ifBody);

  createPlanFunctionParametersPreparationStatements (allDeclarations,
      parallelLoop, ifBody);

  createPlanFunctionCallStatement (allDeclarations, subroutineScope, ifBody);

  createTransferOpDatStatements (ifBody);

  createConvertPositionInPMapsStatements (allDeclarations, parallelLoop,
      subroutineScope, ifBody);

  createConvertPlanFunctionParametersStatements (allDeclarations, parallelLoop,
      subroutineScope, ifBody);

  SgIfStmt * ifStatement =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
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

  fourByteIntegerArrays.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::args);

  fourByteIntegerArrays.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::idxs);

  fourByteIntegerArrays.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::maps);

  fourByteIntegerArrays.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::accesses);

  fourByteIntegerArrays.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::inds);

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

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber);

  fourByteIntegerVariables.push_back (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::iterationCounter);

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

FortranOpenMPHostSubroutineIndirectLoop::FortranOpenMPHostSubroutineIndirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations) :
  FortranOpenMPHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope, moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating OpenMP host subroutine for indirect loop", 2);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
