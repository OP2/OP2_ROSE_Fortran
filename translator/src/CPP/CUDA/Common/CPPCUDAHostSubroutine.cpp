#include <CPPCUDAHostSubroutine.h>
#include <CPPCUDAKernelSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CUDA.h>
#include <CommonNamespaces.h>

void
CPPCUDAHostSubroutine::createReductionEpilogueStatements ()
{
}

void
CPPCUDAHostSubroutine::createReductionPrologueStatements ()
{
}

void
CPPCUDAHostSubroutine::createReductionDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (
      CommonVariableNames::getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::getIterationCounterVariableName (1),
          buildIntType (), subroutineScope));

  variableDeclarations->add (
      CommonVariableNames::getIterationCounterVariableName (2),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::getIterationCounterVariableName (2),
          buildIntType (), subroutineScope));

  variableDeclarations->add (OP2::VariableNames::reductionBytes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::reductionBytes, buildIntType (), subroutineScope));

  variableDeclarations->add (OP2::VariableNames::reductionSharedMemorySize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::reductionSharedMemorySize, buildIntType (),
          subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      string const reductionArrayHostName =
          OP2::VariableNames::getReductionArrayHostName (i);

      Debug::getInstance ()->debugMessage ("Creating host reduction pointer '"
          + reductionArrayHostName + "'", Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgVariableDeclaration * reductionArrayHost =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionArrayHostName, buildPointerType (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope);

      variableDeclarations->add (reductionArrayHostName, reductionArrayHost);
    }
  }
}

void
CPPCUDAHostSubroutine::createCUDAConfigurationLaunchDeclarations ()
{
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration launch local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (CUDA::blocksPerGrid,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::blocksPerGrid, buildIntType (), subroutineScope));

  variableDeclarations->add (CUDA::threadsPerBlock,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::threadsPerBlock, buildIntType (), subroutineScope));

  variableDeclarations->add (CUDA::sharedMemorySize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::sharedMemorySize, buildIntType (), subroutineScope));
}

CPPCUDAHostSubroutine::CPPCUDAHostSubroutine (SgScopeStatement * moduleScope,
    CPPCUDAKernelSubroutine * calleeSubroutine, CPPParallelLoop * parallelLoop) :
  CPPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaHost ();
}
