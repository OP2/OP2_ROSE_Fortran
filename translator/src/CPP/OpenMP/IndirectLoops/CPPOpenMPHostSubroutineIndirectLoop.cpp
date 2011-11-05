#include <CPPOpenMPHostSubroutineIndirectLoop.h>
#include <CPPUserSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <CommonNamespaces.h>
#include <OpenMP.h>

SgStatement *
CPPOpenMPHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
}

SgForStatement *
CPPOpenMPHostSubroutineIndirectLoop::createThreadLoopStatements ()
{
}

SgOmpParallelStatement *
CPPOpenMPHostSubroutineIndirectLoop::createOpenMPLoopStatements ()
{
}

SgBasicBlock *
CPPOpenMPHostSubroutineIndirectLoop::createInitialisePlanFunctionVariablesStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise plan function variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (
            OP2::VariableNames::PlanFunction::args), buildIntVal (i - 1));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, variableDeclarations->getReference (getOpDatName (
            i)));

    appendStatement (assignmentStatement, block);
  }

  map <string, unsigned int> indirectOpDatsToIndirection;
  unsigned int indirection = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (
            OP2::VariableNames::PlanFunction::inds), buildIntVal (i - 1));

    SgExprStatement * assignmentStatement;

    if (parallelLoop->isDirect (i))
    {
      assignmentStatement = buildAssignStatement (arrayIndexExpression,
          buildIntVal (-1));
    }
    else if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isDuplicateOpDat (i))
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
            buildIntVal (indirection));

        indirectOpDatsToIndirection[parallelLoop->getOpDatVariableName (i)]
            = indirection;

        indirection++;
      }
      else
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
            buildIntVal (
                indirectOpDatsToIndirection[parallelLoop->getOpDatVariableName (
                    i)]));
      }
    }

    appendStatement (assignmentStatement, block);
  }

  return block;
}

void
CPPOpenMPHostSubroutineIndirectLoop::createStatements ()
{
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  appendStatementList (
      createInitialisePlanFunctionVariablesStatements ()->getStatementList (),
      subroutineScope);

  appendStatementList (
      createInitialiseNumberOfThreadsStatements ()->getStatementList (),
      subroutineScope);

  appendStatementList (createOpDatTypeCastStatements ()->getStatementList (),
      subroutineScope);

  createReductionPrologueStatements ();

  createReductionEpilogueStatements ();
}

void
CPPOpenMPHostSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;
  using boost::lexical_cast;
  using std::string;

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
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations for plan function",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (
      getIterationCounterVariableName (3),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (3), buildIntType (), subroutineScope));

  variableDeclarations->add (blockID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (blockID,
          buildIntType (), subroutineScope));

  variableDeclarations->add (PlanFunction::blockOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::blockOffset, buildIntType (), subroutineScope));

  variableDeclarations->add (PlanFunction::nblocks,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::nblocks, buildIntType (), subroutineScope));

  variableDeclarations->add (PlanFunction::nelems,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::nelems, buildIntType (), subroutineScope));

  variableDeclarations->add (nbytes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nbytes,
          buildIntType (), subroutineScope));

  variableDeclarations->add (partitionSize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          partitionSize, buildIntType (), subroutineScope));

  variableDeclarations->add (PlanFunction::args,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::args, buildArrayType (buildClassDeclaration (
              OP2::OP_ARG, subroutineScope)->get_type (), buildIntVal (
              parallelLoop->getNumberOfOpDatArgumentGroups ())),
          subroutineScope));

  variableDeclarations->add (PlanFunction::inds,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::inds, buildArrayType (buildIntType (), buildIntVal (
              parallelLoop->getNumberOfOpDatArgumentGroups ())),
          subroutineScope));

  variableDeclarations->add (PlanFunction::planRet,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::planRet, buildPointerType (buildClassDeclaration (
              OP2::OP_PLAN, subroutineScope)->get_type ()), subroutineScope));
}

void
CPPOpenMPHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createOpenMPLocalVariableDeclarations ();

  createOpDatTypeCastVariableDeclarations ();

  createReductionDeclarations ();

  createPlanFunctionDeclarations ();

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementAccessLocalVariableDeclarations ();
  }
}

CPPOpenMPHostSubroutineIndirectLoop::CPPOpenMPHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope, CPPUserSubroutine * calleeSubroutine,
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
