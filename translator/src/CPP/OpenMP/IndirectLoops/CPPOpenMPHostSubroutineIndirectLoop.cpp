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
}

SgForStatement *
CPPOpenMPHostSubroutineIndirectLoop::createThreadLoopStatements ()
{
}

SgOmpParallelStatement *
CPPOpenMPHostSubroutineIndirectLoop::createOpenMPLoopStatements ()
{
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

  createReductionPrologueStatements ();

  createReductionEpilogueStatements ();
}

void
CPPOpenMPHostSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
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

  createReductionDeclarations ();

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
