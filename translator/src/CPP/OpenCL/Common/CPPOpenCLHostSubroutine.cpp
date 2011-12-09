#include "CPPOpenCLHostSubroutine.h"
#include "CPPOpenCLKernelSubroutine.h"
#include "CPPModuleDeclarations.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "OpenCL.h"

void
CPPOpenCLHostSubroutine::createReductionEpilogueStatements ()
{
}

void
CPPOpenCLHostSubroutine::createReductionPrologueStatements ()
{
}

void
CPPOpenCLHostSubroutine::createReductionDeclarations ()
{
}

void
CPPOpenCLHostSubroutine::createOpenCLConfigurationLaunchDeclarations ()
{
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration launch local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (OpenCL::blocksPerGrid,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::blocksPerGrid, OpenCL::getSizeType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::threadsPerBlock,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::threadsPerBlock, OpenCL::getSizeType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::sharedMemorySize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::sharedMemorySize, OpenCL::getSizeType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::errorCode,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::errorCode, OpenCL::getSignedIntegerType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::event,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::event, OpenCL::getEventType (subroutineScope),
          subroutineScope));

  variableDeclarations->add (OpenCL::kernelPointer,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenCL::kernelPointer, OpenCL::getKernelType (subroutineScope),
          subroutineScope));
}

CPPOpenCLHostSubroutine::CPPOpenCLHostSubroutine (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations) :
  CPPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop),
      moduleDeclarations (moduleDeclarations)
{
  variableDeclarations->addVisibilityToSymbolsFromOuterScope (
      moduleDeclarations->getDeclarations ());
}
