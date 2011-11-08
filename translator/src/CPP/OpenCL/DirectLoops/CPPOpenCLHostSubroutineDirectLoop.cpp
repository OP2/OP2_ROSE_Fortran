#include <CPPOpenCLHostSubroutineDirectLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <OpenCL.h>

SgStatement *
CPPOpenCLHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
}

void
CPPOpenCLHostSubroutineDirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  createKernelFunctionCallStatement ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPOpenCLHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
}

CPPOpenCLHostSubroutineDirectLoop::CPPOpenCLHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPOpenCLHostSubroutine (moduleScope, calleeSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
