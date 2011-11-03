#include <FortranOpenMPHostSubroutineIndirectLoop.h>
#include <FortranParallelLoop.h>
#include <FortranOpenMPKernelSubroutine.h>
#include <FortranOpenMPModuleDeclarationsIndirectLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <Debug.h>
#include <OpenMP.h>

SgStatement *
FortranOpenMPHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating kernel function call statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  FortranOpenMPModuleDeclarationsIndirectLoop
      * moduleDeclarationsIndirectLoop =
          static_cast <FortranOpenMPModuleDeclarationsIndirectLoop *> (moduleDeclarations);

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (variableDeclarations->getReference (
      blockID));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isIndirect (i))
    {
      actualParameters->append_expression (
          moduleDeclarationsIndirectLoop->getGlobalOpDatDeclaration (i));

      actualParameters->append_expression (
          moduleDeclarationsIndirectLoop->getLocalToGlobalMappingDeclaration (i));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      actualParameters->append_expression (
          moduleDeclarationsIndirectLoop->getGlobalToLocalMappingDeclaration (i));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false && parallelLoop->isDirect (
        i))
    {
      actualParameters->append_expression (
          moduleDeclarationsIndirectLoop->getGlobalOpDatDeclaration (i));
    }
  }

  actualParameters->append_expression (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::pindSizes));

  actualParameters->append_expression (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::pindOffs));

  actualParameters->append_expression (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::pblkMap));

  actualParameters->append_expression (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::poffset));

  actualParameters->append_expression (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::pnelems));

  actualParameters->append_expression (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::pnthrcol));

  actualParameters->append_expression (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::pthrcol));

  actualParameters->append_expression (variableDeclarations->getReference (
      PlanFunction::blockOffset));

  SgExprStatement * callStatement = buildFunctionCallStmt (
      calleeSubroutine->getSubroutineName (), buildVoidType (),
      actualParameters, subroutineScope);

  return callStatement;
}

void
FortranOpenMPHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating plan function execution statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  FortranOpenMPModuleDeclarationsIndirectLoop
      * moduleDeclarationsIndirectLoop =
          static_cast <FortranOpenMPModuleDeclarationsIndirectLoop *> (moduleDeclarations);

  /*
   * ======================================================
   * Statement to initialise the block offset (before the
   * loops commence execution)
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (PlanFunction::blockOffset),
      buildIntVal (0));

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
      variableDeclarations->getReference (blockID), buildIntVal (0));

  SgExpression * innerLoopUpperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (PlanFunction::nblocks), buildIntVal (
          1));

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

  SgAddOp * addExpression2 = buildAddOp (variableDeclarations->getReference (
      colour1), buildIntVal (1));

  SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::ncolblk), addExpression2);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (PlanFunction::nblocks),
      arrayIndexExpression2);

  appendStatement (assignmentStatement2, outerLoopBody);

  appendStatement (innerLoopStatement, outerLoopBody);

  SgAddOp * addExpression3 = buildAddOp (variableDeclarations->getReference (
      PlanFunction::blockOffset), variableDeclarations->getReference (
      PlanFunction::nblocks));

  SgStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (PlanFunction::blockOffset),
      addExpression3);

  appendStatement (assignmentStatement3, outerLoopBody);

  /*
   * ======================================================
   * ====> Lower and upper bounds
   * ======================================================
   */

  SgAssignOp * outerLoopInitializationExpression = buildAssignOp (
      variableDeclarations->getReference (colour1), buildIntVal (0));

  SgDotExp * dotExpression3 = buildDotExp (
      moduleDeclarationsIndirectLoop->getPlanFunctionDeclaration (
          PlanFunction::actualPlan), buildOpaqueVarRefExp (
          PlanFunction::ncolors, subroutineScope));

  SgExpression * outerLoopUpperBoundExpression = buildSubtractOp (
      dotExpression3, buildIntVal (1));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
}

SgBasicBlock *
FortranOpenMPHostSubroutineIndirectLoop::createTransferOpDatStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (dimension, block));

      SgDotExp * dotExpression2 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (set, block));

      SgDotExp * dotExpression3 = buildDotExp (dotExpression2,
          buildOpaqueVarRefExp (size, block));

      SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
          dotExpression3);

      FortranOpenMPModuleDeclarationsIndirectLoop
          * moduleDeclarationsIndirectLoop =
              static_cast <FortranOpenMPModuleDeclarationsIndirectLoop *> (moduleDeclarations);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          moduleDeclarationsIndirectLoop->getGlobalOpDatSizeDeclaration (i),
          multiplyExpression);

      appendStatement (assignmentStatement, block);

      SgDotExp * parameterExpression1 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (dataOnHost, block));

      SgVarRefExp * parameterExpression2 =
          moduleDeclarations->getGlobalOpDatDeclaration (i);

      SgStatement
          * callStatement =
              FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                  subroutineScope,
                  parameterExpression1,
                  parameterExpression2,
                  FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                      moduleDeclarationsIndirectLoop->getGlobalOpDatSizeDeclaration (
                          i)));

      appendStatement (callStatement, block);
    }
  }

  return block;
}

void
FortranOpenMPHostSubroutineIndirectLoop::createExecutionPlanDeclarations ()
{
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;
  using std::string;
  using std::vector;

  vector <string> fourByteIntegerArrays;

  fourByteIntegerArrays.push_back (PlanFunction::args);

  fourByteIntegerArrays.push_back (PlanFunction::idxs);

  fourByteIntegerArrays.push_back (PlanFunction::maps);

  fourByteIntegerArrays.push_back (PlanFunction::accesses);

  fourByteIntegerArrays.push_back (PlanFunction::inds);

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

  vector <string> fourByteIntegerVariables;

  fourByteIntegerVariables.push_back (getIterationCounterVariableName (1));

  fourByteIntegerVariables.push_back (colour1);

  fourByteIntegerVariables.push_back (PlanFunction::argsNumber);

  fourByteIntegerVariables.push_back (PlanFunction::indsNumber);

  fourByteIntegerVariables.push_back (PlanFunction::blockOffset);

  fourByteIntegerVariables.push_back (PlanFunction::nblocks);

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
  using namespace OP2::VariableNames;

  variableDeclarations->add (OpenMP::numberOfThreads,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::numberOfThreads, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (blockID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          blockID, FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranOpenMPHostSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  initialiseNumberOfThreadsStatements ();

  appendStatement (
      fortranPlan->createPlanFunctionParametersPreparationStatements (),
      subroutineScope);

  SgFunctionCallExp * planFunctionCallExpression =
      fortranPlan->createPlanFunctionCallExpression ();

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      moduleDeclarations->getCPlanReturnDeclaration (),
      planFunctionCallExpression);

  appendStatement (assignmentStatement2, subroutineScope);

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  appendStatement (fortranPlan->createConvertPositionInPMapsStatements (),
      subroutineScope);

  appendStatement (
      fortranPlan->createConvertPlanFunctionParametersStatements (),
      subroutineScope);

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
  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createOpenMPVariableDeclarations ();

  createExecutionPlanDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}

FortranOpenMPHostSubroutineIndirectLoop::FortranOpenMPHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    FortranOpenMPKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations) :
  FortranOpenMPHostSubroutine (moduleScope, kernelSubroutine, parallelLoop,
      moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "OpenMP host subroutine creation for indirect loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  fortranPlan = new FortranPlan (subroutineScope, parallelLoop,
      variableDeclarations);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
