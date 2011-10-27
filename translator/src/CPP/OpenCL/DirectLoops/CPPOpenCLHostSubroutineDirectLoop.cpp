#include <CPPOpenCLHostSubroutineDirectLoop.h>
#include <CPPOpenCLKernelSubroutineDirectLoop.h>
#include <CPPParallelLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CommonNamespaces.h>

void
CPPOpenCLHostSubroutineDirectLoop::createVariableSizesInitialisationStatements ()
{
}

SgStatement *
CPPOpenCLHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
}

void
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelInitialisationStatements ()
{
}

void
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelLocalVariableDeclarationsForDirectLoop ()
{
}

void
CPPOpenCLHostSubroutineDirectLoop::createStatements ()
{
  using namespace SageInterface;

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionPrologueStatements ();
  }

  appendStatement (createKernelFunctionCallStatement (), subroutineScope);

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPOpenCLHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  createOpenCLKernelLocalVariableDeclarationsForDirectLoop ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}

CPPOpenCLHostSubroutineDirectLoop::CPPOpenCLHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * kernelSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPOpenCLHostSubroutine (moduleScope, kernelSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating OpenCL host subroutine for direct loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
