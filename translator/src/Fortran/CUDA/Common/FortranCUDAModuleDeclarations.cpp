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
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      std::cout << "HERE 1\n";

      SgType * baseType = parallelLoop->getOpDatBaseType (i);

      std::cout << "HERE 2\n";

      string const reductionArrayHostName =
          VariableNames::getReductionArrayHostName (i, userSubroutineName);

      Debug::getInstance ()->debugMessage ("Creating host reduction array '"
          + reductionArrayHostName + "'", Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgVariableDeclaration * reductionArrayHost =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionArrayHostName, FortranTypesBuilder::getArray_RankOne (
                  baseType), moduleScope, 1, ALLOCATABLE);

      variableDeclarations->add (reductionArrayHostName, reductionArrayHost);

      string const reductionArrayDeviceName =
          VariableNames::getReductionArrayDeviceName (i, userSubroutineName);

      Debug::getInstance ()->debugMessage ("Creating device reduction array '"
          + reductionArrayDeviceName + "'", Debug::HIGHEST_DEBUG_LEVEL,
          __FILE__, __LINE__);

      SgVariableDeclaration * reductionArrayDevice =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionArrayDeviceName, FortranTypesBuilder::getArray_RankOne (
                  baseType), moduleScope, 2, ALLOCATABLE, DEVICE);

      variableDeclarations->add (reductionArrayDeviceName, reductionArrayDevice);
    }
  }
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
  std::string const & variableName =
      VariableNames::getDimensionsVariableDeclarationName (userSubroutineName);

  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT dimensions declaration '" + variableName + "'",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

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
