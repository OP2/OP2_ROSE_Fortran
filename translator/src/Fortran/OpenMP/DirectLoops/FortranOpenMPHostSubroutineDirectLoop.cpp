#include <boost/lexical_cast.hpp>
#include <FortranOpenMPHostSubroutineDirectLoop.h>
#include <CommonNamespaces.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <ROSEHelper.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

std::string
FortranOpenMPHostSubroutineDirectLoop::getLocalOPDATVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + IndirectAndDirectLoop::Fortran::VariableSuffixes::local;
}

SgStatement *
FortranOpenMPHostSubroutineDirectLoop::createKernelCall ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildNullExpression;

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        actualParameters->append_expression (buildVarRefExp (
            moduleDeclarations->getGlobalOPDATDeclaration (i)));
      }
      else
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
            localVariableDeclarations[OpenMP::threadIndex]), buildIntVal (64));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (),
                multiplyExpression, buildNullExpression (), buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression = buildPntrArrRefExp (
            buildVarRefExp (
                localVariableDeclarations[getLocalOPDATVariableName (i)]),
            buildExprListExp (arraySubscriptExpression));

        actualParameters->append_expression (parameterExpression);
      }
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
          initializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  appendStatement (doStatement, subroutineScope);

  addTextForUnparser (doStatement, "!$OMP PARALLEL DO PRIVATE("
      + OpenMP::sliceStart + ", " + OpenMP::sliceEnd + ")\n",
      AstUnparseAttribute::e_before);

  addTextForUnparser (doStatement, "!$OMP END PARALLEL DO\n",
      AstUnparseAttribute::e_after);
}

void
FortranOpenMPHostSubroutineDirectLoop::createReductionEpilogueStatements ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * Create the inner loop
   * ======================================================
   */

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->isReductionRequired (i))
    {
      SgAssignOp
          * innerLoopInitializationExpression =
              buildAssignOp (
                  buildVarRefExp (
                      localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2]),
                  buildIntVal (0));

      SgSubtractOp * innerLoopUpperBoundExpression =
          buildSubtractOp (
              buildIntVal (parallelLoop->get_OP_DAT_Dimension (i)),
              buildIntVal (1));

      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgMultiplyOp
          * multiplyExpression1 =
              buildMultiplyOp (
                  buildVarRefExp (
                      localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1]),
                  buildIntVal (64));

      SgAddOp
          * addExpression1 =
              buildAddOp (
                  buildVarRefExp (
                      localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2]),
                  multiplyExpression1);

      SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
          buildVarRefExp (localVariableDeclarations[getLocalOPDATVariableName (
              i)]), addExpression1);

      SgAddOp
          * addExpression2 =
              buildAddOp (
                  buildIntVal (parallelLoop->get_OP_DAT_Dimension (i)),
                  buildVarRefExp (
                      localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2]));

      SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
          buildVarRefExp (moduleDeclarations->getGlobalOPDATDeclaration (i)),
          addExpression2);

      SgAddOp
          * addExpression3 =
              buildAddOp (
                  buildIntVal (parallelLoop->get_OP_DAT_Dimension (i)),
                  buildVarRefExp (
                      localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2]));

      SgPntrArrRefExp * arrayIndexExpression3 = buildPntrArrRefExp (
          buildVarRefExp (moduleDeclarations->getGlobalOPDATDeclaration (i)),
          addExpression3);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayIndexExpression3, buildAddOp (arrayIndexExpression1,
              arrayIndexExpression2));

      innerLoopBody->append_statement (assignmentStatement);

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopInitializationExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      outerLoopBody->append_statement (innerLoopStatement);
    }
  }

  /*
   * ======================================================
   * Create the outer loop
   * ======================================================
   */

  SgAssignOp
      * outerLoopInitializationExpression =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1]),
              buildIntVal (0));

  SgSubtractOp * outerLoopUpperBoundExpression = buildSubtractOp (
      buildVarRefExp (localVariableDeclarations[OpenMP::numberOfThreads]),
      buildIntVal (1));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
}

void
FortranOpenMPHostSubroutineDirectLoop::createReductionPrologueStatements ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * Create the inner loop
   * ======================================================
   */

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->isReductionRequired (i))
    {
      SgAssignOp
          * innerLoopInitializationExpression =
              buildAssignOp (
                  buildVarRefExp (
                      localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2]),
                  buildIntVal (0));

      SgSubtractOp * innerLoopUpperBoundExpression =
          buildSubtractOp (
              buildIntVal (parallelLoop->get_OP_DAT_Dimension (i)),
              buildIntVal (1));

      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgMultiplyOp
          * multiplyExpression =
              buildMultiplyOp (
                  buildVarRefExp (
                      localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1]),
                  buildIntVal (64));

      SgAddOp
          * addExpression =
              buildAddOp (
                  buildVarRefExp (
                      localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2]),
                  multiplyExpression);

      SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
          buildVarRefExp (localVariableDeclarations[getLocalOPDATVariableName (
              i)]), addExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayIndexExpression, buildIntVal (0));

      innerLoopBody->append_statement (assignmentStatement);

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopInitializationExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      outerLoopBody->append_statement (innerLoopStatement);
    }
  }

  /*
   * ======================================================
   * Create the outer loop
   * ======================================================
   */

  SgAssignOp
      * outerLoopInitializationExpression =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1]),
              buildIntVal (0));

  SgSubtractOp * outerLoopUpperBoundExpression = buildSubtractOp (
      buildVarRefExp (localVariableDeclarations[OpenMP::numberOfThreads]),
      buildIntVal (1));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
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

      SgDotExp * dotExpression2;

      if (parallelLoop->isReductionRequired (i) == false)
      {
        dotExpression2 = buildDotExp (buildVarRefExp (
            formalParameterDeclarations[get_OP_SET_FormalParameterName ()]),
            buildOpaqueVarRefExp (
                IndirectAndDirectLoop::Fortran::HostSubroutine::size,
                subroutineScope));
      }
      else
      {
        dotExpression2 = buildDotExp (buildVarRefExp (
            formalParameterDeclarations[get_OP_DAT_FormalParameterName (i)]),
            buildOpaqueVarRefExp (
                IndirectAndDirectLoop::Fortran::HostSubroutine::dim,
                subroutineScope));
      }

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
FortranOpenMPHostSubroutineDirectLoop::createReductionLocalVariableDeclarations ()
{
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false
        && parallelLoop->isReductionRequired (i))
    {
      SgArrayType * arrayType = isSgArrayType (
          parallelLoop->get_OP_DAT_Type (i));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildIntVal (64),
          buildIntVal (64));

      SgAddOp * addExpression = buildAddOp (buildIntVal (
          parallelLoop->get_OP_DAT_Dimension (i)), multiplyExpression);

      SgSubtractOp * subtractExpression = buildSubtractOp (addExpression,
          buildIntVal (parallelLoop->get_OP_DAT_Dimension (i)));

      SgType * newArrayType =
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              arrayType->get_base_type (), buildIntVal (0), subtractExpression);

      localVariableDeclarations[getLocalOPDATVariableName (i)]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              getLocalOPDATVariableName (i), newArrayType, subroutineScope);
    }
  }

  localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);
}

void
FortranOpenMPHostSubroutineDirectLoop::createStatements ()
{
  initialiseThreadVariables ();

  createCToFortranPointerConversionStatements ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  createKernelDoLoop ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
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

FortranOpenMPHostSubroutineDirectLoop::FortranOpenMPHostSubroutineDirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranOpenMPModuleDeclarationsDirectLoop * moduleDeclarations) :
  FortranOpenMPHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope)
{
  this->moduleDeclarations = moduleDeclarations;

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
