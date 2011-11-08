#include <CPPCUDAHostSubroutine.h>
#include <CPPCUDAKernelSubroutine.h>
#include <CPPModuleDeclarations.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CUDA.h>
#include <CommonNamespaces.h>

void
CPPCUDAHostSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);
}

void
CPPCUDAHostSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (reductionBytes), buildIntVal (0));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (reductionSharedMemorySize),
      buildIntVal (0));

  appendStatement (assignmentStatement2, subroutineScope);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      SgSizeOfOp * sizeOfExpression = buildSizeOfOp (
          parallelLoop->getOpDatBaseType (i));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          variableDeclarations->getReference (CUDA::blocksPerGrid),
          sizeOfExpression);

      SgFunctionCallExp * roundUpCallExpression;

      if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
      {
        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
            multiplyExpression1, buildIntVal (parallelLoop->getOpDatDimension (
                i)));

        roundUpCallExpression
            = OP2::CPPMacroSupport::createRoundUpCallStatement (
                subroutineScope, multiplyExpression2);
      }
      else
      {
        roundUpCallExpression
            = OP2::CPPMacroSupport::createRoundUpCallStatement (
                subroutineScope, multiplyExpression1);
      }

      SgPlusAssignOp * assignmentStatement3 = buildPlusAssignOp (
          variableDeclarations->getReference (reductionBytes),
          roundUpCallExpression);

      appendStatement (buildExprStatement (assignmentStatement3),
          subroutineScope);

      SgFunctionCallExp * maxCallExpression =
          OP2::CPPMacroSupport::createMaxCallStatement (subroutineScope,
              variableDeclarations->getReference (reductionSharedMemorySize),
              sizeOfExpression);

      SgExprStatement * assignmentStatement4 = buildAssignStatement (
          variableDeclarations->getReference (reductionSharedMemorySize),
          maxCallExpression);

      appendStatement (assignmentStatement4, subroutineScope);
    }
  }

  SgFunctionCallExp * reallocateReductionArraysExpression =
      CUDA::CPPRuntimeSupport::getReallocateReductionArraysCallStatement (
          subroutineScope, variableDeclarations->getReference (reductionBytes));

  appendStatement (buildExprStatement (reallocateReductionArraysExpression),
      subroutineScope);

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      variableDeclarations->getReference (reductionBytes), buildIntVal (0));

  appendStatement (assignmentStatement5, subroutineScope);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
      {
      }
      else
      {
      }
    }
  }
}

void
CPPCUDAHostSubroutine::createReductionDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));

  variableDeclarations->add (
      getIterationCounterVariableName (2),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2), buildIntType (), subroutineScope));

  variableDeclarations->add (reductionBytes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          reductionBytes, buildIntType (), subroutineScope));

  variableDeclarations->add (reductionSharedMemorySize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          reductionSharedMemorySize, buildIntType (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      string const reductionArrayHostName = getReductionArrayHostName (i);

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
    CPPCUDAKernelSubroutine * calleeSubroutine, CPPParallelLoop * parallelLoop,
    CPPModuleDeclarations * moduleDeclarations) :
  CPPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop),
      moduleDeclarations (moduleDeclarations)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaHost ();
}
