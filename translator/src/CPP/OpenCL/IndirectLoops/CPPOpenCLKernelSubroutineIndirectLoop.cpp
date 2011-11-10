#include <CPPOpenCLKernelSubroutineIndirectLoop.h>
#include <CPPOpenCLUserSubroutine.h>
#include <CPPParallelLoop.h>
#include <RoseHelper.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CompilerGeneratedNames.h>
#include <Debug.h>
#include <boost/lexical_cast.hpp>

SgStatement *
CPPOpenCLKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createPointeredIncrementsOrWritesStatements ()
{
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createInnerExecutionLoopStatements (
    SgScopeStatement * scope)
{

}

void
CPPOpenCLKernelSubroutineIndirectLoop::createIncrementAdjustmentStatements (
    SgScopeStatement * scope)
{
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createInitialiseLocalOpDatStatements (
    SgScopeStatement * scope)
{

}

void
CPPOpenCLKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
{

}

void
CPPOpenCLKernelSubroutineIndirectLoop::createThreadZeroStatements ()
{
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createStatements ()
{
  createThreadZeroStatements ();

  createExecutionLoopStatements ();

  createPointeredIncrementsOrWritesStatements ();
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
}

CPPOpenCLKernelSubroutineIndirectLoop::CPPOpenCLKernelSubroutineIndirectLoop (
    SgScopeStatement * moduleScope, CPPOpenCLUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPOpenCLKernelSubroutine (moduleScope, userSubroutine, parallelLoop,
      reductionSubroutines)
{
  Debug::getInstance ()->debugMessage ("<Kernel, Indirect, OpenCL>",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
