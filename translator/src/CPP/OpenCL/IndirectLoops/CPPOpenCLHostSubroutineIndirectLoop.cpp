#include <CPPOpenCLHostSubroutineIndirectLoop.h>
#include <CPPParallelLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CompilerGeneratedNames.h>
#include <RoseHelper.h>
#include <Debug.h>

SgStatement *
CPPOpenCLHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
}

void
CPPOpenCLHostSubroutineIndirectLoop::createStatements ()
{
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
CPPOpenCLHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  if (parallelLoop->isReductionRequired ())
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

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
