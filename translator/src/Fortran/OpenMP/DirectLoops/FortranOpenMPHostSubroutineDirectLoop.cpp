#include <FortranOpenMPHostSubroutineDirectLoop.h>
#include <CommonNamespaces.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranOpenMPHostSubroutineDirectLoop::createKernelCall ()
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprStatement;

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      actualParameters->append_expression (buildVarRefExp (
          moduleDeclarations->getGlobalOPDATDeclaration (i)));
    }
  }

  actualParameters->append_expression (buildVarRefExp (
      localVariableDeclarations[OpenMP::sliceStart]));

  actualParameters->append_expression (buildVarRefExp (
      localVariableDeclarations[OpenMP::sliceEnd]));

  SgFunctionSymbol * kernelSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine (kernelSubroutineName,
          subroutineScope);

  SgFunctionCallExp * kernelCall = buildFunctionCallExp (kernelSymbol,
      actualParameters);

  return buildExprStatement (kernelCall);
}

void
FortranOpenMPHostSubroutineDirectLoop::createKernelDoLoop ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  SgAssignOp * initializationExpression = buildAssignOp (buildVarRefExp (
      localVariableDeclarations[OpenMP::threadIndex]), buildIntVal (0));

  SgSubtractOp * upperBoundExpression = buildSubtractOp (buildVarRefExp (
      localVariableDeclarations[OpenMP::numberOfThreads]), buildIntVal (1));

  SgIntVal * strideExpression = buildIntVal (1);

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgDotExp * dotExpression1 =
      buildDotExp (buildVarRefExp (
          formalParameterDeclarations[get_OP_SET_FormalParameterName ()]),
          buildOpaqueVarRefExp (
              IndirectAndDirectLoop::Fortran::HostSubroutine::size,
              subroutineScope));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (dotExpression1,
      buildVarRefExp (localVariableDeclarations[OpenMP::threadIndex]));

  SgDivideOp * divideExpression1 = buildDivideOp (multiplyExpression1,
      buildVarRefExp (localVariableDeclarations[OpenMP::numberOfThreads]));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (localVariableDeclarations[OpenMP::sliceStart]),
      divideExpression1);

  SgDotExp * dotExpression2 =
      buildDotExp (buildVarRefExp (
          formalParameterDeclarations[get_OP_SET_FormalParameterName ()]),
          buildOpaqueVarRefExp (
              IndirectAndDirectLoop::Fortran::HostSubroutine::size,
              subroutineScope));

  SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
      localVariableDeclarations[OpenMP::threadIndex]), buildIntVal (1));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (dotExpression2,
      addExpression2);

  SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression2,
      buildVarRefExp (localVariableDeclarations[OpenMP::numberOfThreads]));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (localVariableDeclarations[OpenMP::sliceEnd]),
      divideExpression2);

  loopBody->append_statement (assignmentStatement1);

  loopBody->append_statement (assignmentStatement2);

  loopBody->append_statement (createKernelCall ());

  SgFortranDo * doStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, strideExpression,
          loopBody);

  appendStatement (doStatement, subroutineScope);

  addTextForUnparser (doStatement, "!$OMP PARALLEL DO PRIVATE("
      + OpenMP::sliceStart + ", " + OpenMP::sliceEnd + ")\n",
      AstUnparseAttribute::e_before);

  addTextForUnparser (doStatement, "!$OMP END PARALLEL DO\n",
      AstUnparseAttribute::e_after);
}

void
FortranOpenMPHostSubroutineDirectLoop::createCToFortranPointerConversionStatements ()
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

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (true));

  SgBasicBlock * ifBody = buildBasicBlock ();

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
          formalParameterDeclarations[get_OP_DAT_FormalParameterName (i)]),
          buildOpaqueVarRefExp (
              IndirectAndDirectLoop::Fortran::HostSubroutine::dat,
              subroutineScope));

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          formalParameterDeclarations[get_OP_SET_FormalParameterName ()]),
          buildOpaqueVarRefExp (
              IndirectAndDirectLoop::Fortran::HostSubroutine::size,
              subroutineScope));

      SgStatement * cToFortranCallStatement = createCToFortranPointerCall (
          dotExpression1, buildVarRefExp (
              formalParameterDeclarations[get_OP_DAT_FormalParameterName (i)]),
          buildOpaqueVarRefExp ("(/" + dotExpression2->unparseToString ()
              + "/)", subroutineScope));

      ifBody->append_statement (cToFortranCallStatement);
    }
  }

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (
          moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (false));

  ifBody->append_statement (assignmentStatement1);

  SgIfStmt * ifStatement =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, subroutineScope);
}

void
FortranOpenMPHostSubroutineDirectLoop::initialiseThreadVariables ()
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (localVariableDeclarations[OpenMP::threadIndex]),
      buildIntVal (-1));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (localVariableDeclarations[OpenMP::sliceStart]),
      buildIntVal (-1));

  appendStatement (assignmentStatement2, subroutineScope);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      buildVarRefExp (localVariableDeclarations[OpenMP::sliceEnd]),
      buildIntVal (-1));

  appendStatement (assignmentStatement3, subroutineScope);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("omp_get_max_threads",
          subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      buildExprListExp ());

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      buildVarRefExp (localVariableDeclarations[OpenMP::numberOfThreads]),
      functionCall);

  appendStatement (assignmentStatement4, subroutineScope);

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      buildVarRefExp (localVariableDeclarations[OpenMP::numberOfThreads]),
      buildIntVal (1));

  appendStatement (assignmentStatement5, subroutineScope);

  addTextForUnparser (assignmentStatement4, "#ifdef _OPENMP\n",
      AstUnparseAttribute::e_after);

  addTextForUnparser (assignmentStatement5, "#else\n",
      AstUnparseAttribute::e_before);

  addTextForUnparser (assignmentStatement5, "#endif\n",
      AstUnparseAttribute::e_after);
}

void
FortranOpenMPHostSubroutineDirectLoop::createStatements ()
{
  initialiseThreadVariables ();

  createCToFortranPointerConversionStatements ();

  createKernelDoLoop ();
}

void
FortranOpenMPHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  localVariableDeclarations[OpenMP::numberOfThreads]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::numberOfThreads, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope);

  localVariableDeclarations[OpenMP::threadIndex]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::threadIndex, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope);

  localVariableDeclarations[OpenMP::sliceStart]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::sliceStart, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope);

  localVariableDeclarations[OpenMP::sliceEnd]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::sliceEnd, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope);
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranOpenMPHostSubroutineDirectLoop::FortranOpenMPHostSubroutineDirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranOpenMPModuleDeclarations * moduleDeclarations) :
  FortranOpenMPHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope, moduleDeclarations)
{
  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
