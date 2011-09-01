#include <FortranOpenMPHostSubroutineDirectLoop.h>
#include <CommonNamespaces.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranOpenMPHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
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

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        actualParameters->append_expression (buildVarRefExp (
            moduleDeclarations->getGlobalOpDatDeclaration (i)));
      }
      else
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (OpenMP::threadIndex)), buildIntVal (64));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                multiplyExpression, buildNullExpression (), buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            RoseHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                VariableNames::getOpDatLocalName (i))), buildExprListExp (
                arraySubscriptExpression));

        actualParameters->append_expression (parameterExpression);
      }
    }
  }

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OpenMP::sliceStart)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OpenMP::sliceEnd)));

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

  Debug::getInstance ()->debugMessage ("Creating kernel do loop",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgAssignOp * initializationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (OpenMP::threadIndex)), buildIntVal (0));

  SgSubtractOp * upperBoundExpression = buildSubtractOp (buildVarRefExp (
      variableDeclarations->get (OpenMP::numberOfThreads)), buildIntVal (1));

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (VariableNames::getOpSetName ())),
      buildOpaqueVarRefExp (CommonVariableNames::size, subroutineScope));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (dotExpression1,
      buildVarRefExp (variableDeclarations->get (OpenMP::threadIndex)));

  SgDivideOp * divideExpression1 = buildDivideOp (multiplyExpression1,
      buildVarRefExp (variableDeclarations->get (OpenMP::numberOfThreads)));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OpenMP::sliceStart)),
      divideExpression1);

  SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
      variableDeclarations->get (VariableNames::getOpSetName ())),
      buildOpaqueVarRefExp (CommonVariableNames::size, subroutineScope));

  SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (OpenMP::threadIndex)), buildIntVal (1));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (dotExpression2,
      addExpression2);

  SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression2,
      buildVarRefExp (variableDeclarations->get (OpenMP::numberOfThreads)));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OpenMP::sliceEnd)),
      divideExpression2);

  loopBody->append_statement (assignmentStatement1);

  loopBody->append_statement (assignmentStatement2);

  loopBody->append_statement (createKernelFunctionCallStatement ());

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

SgBasicBlock *
FortranOpenMPHostSubroutineDirectLoop::createTransferOpDatStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating statements to transfer OP_DAT", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * parameterExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (CommonVariableNames::dat, block));

      SgVarRefExp * parameterExpression2 = buildVarRefExp (
          moduleDeclarations->getGlobalOpDatDeclaration (i));

      SgDotExp * parameterExpression3;

      if (parallelLoop->isReductionRequired (i) == false)
      {
        parameterExpression3 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpSetName ())),
            buildOpaqueVarRefExp (CommonVariableNames::size, block));
      }
      else
      {
        parameterExpression3 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatName (i))),
            buildOpaqueVarRefExp (CommonVariableNames::dim, block));
      }

      SgStatement * callStatement =
          SubroutineCalls::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpression1, parameterExpression2,
              buildOpaqueVarRefExp ("(/"
                  + parameterExpression3->unparseToString () + "/)", block));

      appendStatement (callStatement, block);
    }
  }

  return block;
}

void
FortranOpenMPHostSubroutineDirectLoop::createFirstTimeExecutionStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildBoolValExp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating statements for first execution of host subroutine",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * ifBody = buildBasicBlock ();

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (false));

  appendStatement (assignmentStatement, ifBody);

  appendStatement (createTransferOpDatStatements (), ifBody);

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      moduleDeclarations->getFirstExecutionBooleanDeclaration ()),
      buildBoolValExp (true));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, subroutineScope);
}

void
FortranOpenMPHostSubroutineDirectLoop::initialiseThreadVariablesStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise thread variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OpenMP::threadIndex)),
      buildIntVal (-1));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OpenMP::sliceStart)),
      buildIntVal (-1));

  appendStatement (assignmentStatement2, subroutineScope);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OpenMP::sliceEnd)),
      buildIntVal (-1));

  appendStatement (assignmentStatement3, subroutineScope);
}

void
FortranOpenMPHostSubroutineDirectLoop::createOpenMPVariableDeclarations ()
{
  variableDeclarations->add (OpenMP::numberOfThreads,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::numberOfThreads, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (OpenMP::threadIndex,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::threadIndex, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (OpenMP::sliceStart,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::sliceStart, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (OpenMP::sliceEnd,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::sliceEnd, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));
}

void
FortranOpenMPHostSubroutineDirectLoop::createStatements ()
{
  initialiseThreadVariablesStatements ();

  initialiseNumberOfThreadsStatements ();

  createFirstTimeExecutionStatements ();

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
  createOpenMPVariableDeclarations ();

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
    std::string const & kernelSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranOpenMPModuleDeclarationsDirectLoop * moduleDeclarations) :
  FortranOpenMPHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope, moduleDeclarations)
{
  Debug::getInstance ()->debugMessage ("<Host, Direct, OpenMP>",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
