#include <CPPCUDAHostSubroutineDirectLoop.h>
#include <CPPParallelLoop.h>
#include <CPPKernelSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>

void
CPPCUDAHostSubroutineDirectLoop::createCUDAKernelInitialisationStatements ()
{
}

SgStatement *
CPPCUDAHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
}

void
CPPCUDAHostSubroutineDirectLoop::createStatements ()
{
}

void
CPPCUDAHostSubroutineDirectLoop::createCUDAKernelActualParameterDeclarations ()
{
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (OP2::VariableNames::sharedMemoryOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::sharedMemoryOffset, buildIntType (),
          subroutineScope));

  variableDeclarations->add (OP2::VariableNames::warpSize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::warpSize, buildIntType (), subroutineScope));
}

void
CPPCUDAHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  createCUDAConfigurationLaunchDeclarations ();

  createCUDAKernelActualParameterDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}

CPPCUDAHostSubroutineDirectLoop::CPPCUDAHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope, CPPCUDAKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPCUDAHostSubroutine (moduleScope, calleeSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
