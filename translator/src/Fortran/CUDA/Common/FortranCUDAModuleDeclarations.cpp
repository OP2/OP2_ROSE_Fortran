#include <FortranCUDAModuleDeclarations.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranCUDAReductionSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

void
FortranCUDAModuleDeclarations::createReductionDeclarations ()
{
  SgVariableDeclaration * variableDeclaration1 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          ReductionSubroutine::reductionArrayHost,
          parallelLoop->getReductionType (), moduleScope, 1, ALLOCATABLE);

  variableDeclarations->add (ReductionSubroutine::reductionArrayHost,
      variableDeclaration1);

  SgVariableDeclaration * variableDeclaration2 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          ReductionSubroutine::reductionArrayDevice,
          parallelLoop->getReductionType (), moduleScope, 2, ALLOCATABLE,
          DEVICE);

  variableDeclarations->add (ReductionSubroutine::reductionArrayDevice,
      variableDeclaration2);
}

void
FortranCUDAModuleDeclarations::createDataSizesDeclaration ()
{
  Debug::getInstance ()->debugMessage (
      "Generating data sizes declaration at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  std::string const & variableName =
      VariableNames::getDataSizesVariableDeclarationName (userSubroutineName);

  SgVariableDeclaration * variableDeclaration =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName, dataSizesDeclaration->getType (), moduleScope, 1,
          DEVICE);

  variableDeclarations->add (variableName, variableDeclaration);
}

void
FortranCUDAModuleDeclarations::createDimensionsDeclaration ()
{
  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT dimensions declaration at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  std::string const & variableName =
      VariableNames::getDimensionsVariableDeclarationName (userSubroutineName);

  SgVariableDeclaration * variableDeclaration =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName, dimensionsDeclaration->getType (), moduleScope, 1,
          DEVICE);

  variableDeclarations->add (variableName, variableDeclaration);
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgVariableDeclaration *
FortranCUDAModuleDeclarations::getReductionArrayHostVariableDeclaration ()
{
  return variableDeclarations->get (ReductionSubroutine::reductionArrayHost);
}

SgVariableDeclaration *
FortranCUDAModuleDeclarations::getReductionArrayDeviceVariableDeclaration ()
{
  return variableDeclarations->get (ReductionSubroutine::reductionArrayDevice);
}

SgVariableDeclaration *
FortranCUDAModuleDeclarations::getDataSizesVariableDeclaration ()
{
  std::string const & variableName =
      VariableNames::getDataSizesVariableDeclarationName (userSubroutineName);

  return variableDeclarations->get (variableName);
}

SgVariableDeclaration *
FortranCUDAModuleDeclarations::getDimensionsVariableDeclaration ()
{
  std::string const & variableName =
      VariableNames::getDimensionsVariableDeclarationName (userSubroutineName);

  return variableDeclarations->get (variableName);
}

FortranCUDAModuleDeclarations::FortranCUDAModuleDeclarations (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranCUDADataSizesDeclaration * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration) :
  FortranModuleDeclarations (userSubroutineName, parallelLoop, moduleScope),
      dataSizesDeclaration (dataSizesDeclaration), dimensionsDeclaration (
          dimensionsDeclaration)
{
  Debug::getInstance ()->debugMessage (
      "Generating CUDA module scope declarations", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createDataSizesDeclaration ();

  createDimensionsDeclaration ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}
