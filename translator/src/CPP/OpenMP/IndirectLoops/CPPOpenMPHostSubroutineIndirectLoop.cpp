#include <CPPOpenMPHostSubroutineIndirectLoop.h>
#include <CPPUserSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <CompilerGeneratedNames.h>
#include <OP2Definitions.h>
#include <OpenMP.h>

SgStatement *
CPPOpenMPHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statement to call OpenMP kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i))
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (64));

        SgAddOp * addExpression = buildAddOp (
            variableDeclarations->getReference (getReductionArrayHostName (i)),
            multiplyExpression);

        actualParameters->append_expression (addExpression);
      }
      else
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (getOpDatLocalName (i)));
      }
    }
  }

  unsigned int arrayIndex = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
            ind_maps, buildArrayType (buildIntType ()), NULL, subroutineScope);

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclaration), buildIntVal (arrayIndex));

        SgArrowExp * arrowExpression = buildArrowExp (
            variableDeclarations->getReference (planRet), arrayExpression);

        actualParameters->append_expression (arrowExpression);

        arrayIndex++;
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          loc_maps, buildArrayType (buildIntType ()), NULL, subroutineScope);

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclaration), buildIntVal (i - 1));

      SgArrowExp * arrowExpression = buildArrowExp (
          variableDeclarations->getReference (planRet), arrayExpression);

      actualParameters->append_expression (arrowExpression);
    }
  }

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          ind_sizes, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          ind_offs, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          blkmap, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          offset, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          nelems, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          thrcol, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          nthrcol, subroutineScope)));

  actualParameters->append_expression (variableDeclarations->getReference (
      blockOffset));

  actualParameters->append_expression (variableDeclarations->getReference (
      blockID));

  SgFunctionCallExp * functionCallExpression = buildFunctionCallExp (
      calleeSubroutine->getSubroutineName (), buildVoidType (),
      actualParameters, subroutineScope);

  return buildExprStatement (functionCallExpression);
}

void
CPPOpenMPHostSubroutineIndirectLoop::createOpenMPLoopStatements (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace OpenMP;

  Debug::getInstance ()->debugMessage ("Creating OpenMP for loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (createKernelFunctionCallStatement (), loopBody);

  /*
   * ======================================================
   * For loop statement
   * ======================================================
   */

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (blockID), buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (blockID),
      variableDeclarations->getReference (nblocks));

  SgPlusPlusOp * strideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (blockID));

  SgForStatement * forLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (forLoopStatement, scope);

  /*
   * ======================================================
   * The OpenMP directive
   * ======================================================
   */

  std::vector <SgVarRefExp *> privateVariableReferences;

  privateVariableReferences.push_back (variableDeclarations->getReference (
      blockID));

  addTextForUnparser (forLoopStatement,
      OpenMP::getParallelLoopDirectiveString () + OpenMP::getPrivateClause (
          privateVariableReferences), AstUnparseAttribute::e_before);
}

SgBasicBlock *
CPPOpenMPHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace OpenMP;

  Debug::getInstance ()->debugMessage (
      "Creating plan function execution loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (blockOffset), buildIntVal (0));

  appendStatement (assignmentStatement1, block);

  /*
   * ======================================================
   * For loop body
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (ncolblk, subroutineScope),
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgArrowExp * arrowExpression2 = buildArrowExp (
      variableDeclarations->getReference (planRet), arrayExpression2);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (nblocks), arrowExpression2);

  appendStatement (assignmentStatement2, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  createOpenMPLoopStatements (loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement5 = buildExprStatement (
      buildPlusAssignOp (variableDeclarations->getReference (blockOffset),
          variableDeclarations->getReference (nblocks)));

  appendStatement (assignmentStatement5, loopBody);

  /*
   * ======================================================
   * For loop statement
   * ======================================================
   */

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgArrowExp * arrowExpression = buildArrowExp (
      variableDeclarations->getReference (planRet), buildOpaqueVarRefExp (
          ncolors, subroutineScope));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      arrowExpression);

  SgPlusPlusOp * strideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgForStatement * forLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (forLoopStatement, block);

  return block;
}

void
CPPOpenMPHostSubroutineIndirectLoop::createStatements ()
{
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  appendStatementList (
      createInitialisePlanFunctionArrayStatements ()->getStatementList (),
      subroutineScope);

  appendStatementList (
      createInitialiseNumberOfThreadsStatements ()->getStatementList (),
      subroutineScope);

  appendStatementList (createOpDatTypeCastStatements ()->getStatementList (),
      subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  appendStatementList (
      createPlanFunctionExecutionStatements ()->getStatementList (),
      subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPOpenMPHostSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for incremented OP_DATS",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (numberOfColours,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfColours, buildIntType (), subroutineScope));

  variableDeclarations->add (nelems2,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nelems2,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (colour1,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (colour1,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (colour2,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (colour2,
          buildIntType (), subroutineScope));
}

void
CPPOpenMPHostSubroutineIndirectLoop::createPlanFunctionDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations for plan function",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (
      getIterationCounterVariableName (4),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (4), buildIntType (), subroutineScope));

  variableDeclarations->add (blockID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (blockID,
          buildIntType (), subroutineScope));

  variableDeclarations->add (blockOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          blockOffset, buildIntType (), subroutineScope));

  variableDeclarations->add (nblocks,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nblocks,
          buildIntType (), subroutineScope));

  variableDeclarations->add (nelems,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nelems,
          buildIntType (), subroutineScope));

  variableDeclarations->add (nbytes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nbytes,
          buildIntType (), subroutineScope));

  variableDeclarations->add (partitionSize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          partitionSize, buildIntType (), subroutineScope));

  variableDeclarations->add (args,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (args,
          buildArrayType (buildOpaqueType (OP2::OP_ARG, subroutineScope),
              buildIntVal (parallelLoop->getNumberOfOpDatArgumentGroups ())),
          subroutineScope));

  variableDeclarations->add (inds,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (inds,
          buildArrayType (buildIntType (), buildIntVal (
              parallelLoop->getNumberOfOpDatArgumentGroups ())),
          subroutineScope));

  variableDeclarations->add (planRet,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (planRet,
          buildPointerType (buildOpaqueType (OP2::OP_PLAN, subroutineScope)),
          subroutineScope));
}

void
CPPOpenMPHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createOpenMPLocalVariableDeclarations ();

  createOpDatTypeCastVariableDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }

  createPlanFunctionDeclarations ();

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementAccessLocalVariableDeclarations ();
  }
}

CPPOpenMPHostSubroutineIndirectLoop::CPPOpenMPHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    CPPOpenMPKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPOpenMPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
