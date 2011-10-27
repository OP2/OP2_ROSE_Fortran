#include <CPPOpenCLHostSubroutineIndirectLoop.h>
#include <CPPPlan.h>
#include <CPPParallelLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <Debug.h>

SgStatement *
CPPOpenCLHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
}

void
CPPOpenCLHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
}

void
CPPOpenCLHostSubroutineIndirectLoop::createExecutionPlanDeclarations ()
{
}

void
CPPOpenCLHostSubroutineIndirectLoop::createStatements ()
{
  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionPrologueStatements ();
  }

  createPlanFunctionExecutionStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPOpenCLHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  createExecutionPlanDeclarations ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionDeclarations ();
  }
}

CPPOpenCLHostSubroutineIndirectLoop::CPPOpenCLHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * kernelSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPOpenCLHostSubroutine (moduleScope, kernelSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating OpenCL host subroutine for indirect loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  plan = new CPPPlan (subroutineScope, parallelLoop, variableDeclarations);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
