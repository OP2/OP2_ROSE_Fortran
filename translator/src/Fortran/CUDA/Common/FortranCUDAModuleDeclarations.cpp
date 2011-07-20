#include <FortranCUDAModuleDeclarations.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

void
FortranCUDAModuleDeclarations::createDataSizesDeclaration ()
{
  Debug::getInstance ()->debugMessage (
      "Generating data sizes declaration at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::argsSizes, dataSizesDeclaration->getType (),
          moduleScope, 1, DEVICE);

  variableDeclarations->add (CommonVariableNames::argsSizes,
      variableDeclaration);
}

void
FortranCUDAModuleDeclarations::createDimensionsDeclaration ()
{
  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT dimensions declaration at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::opDatDimensions,
          dimensionsDeclaration->getType (), moduleScope, 1, DEVICE);

  variableDeclarations->add (CommonVariableNames::opDatDimensions,
      variableDeclaration);
}

SgVariableDeclaration *
FortranCUDAModuleDeclarations::getDataSizesVariableDeclaration ()
{
  return variableDeclarations->get (CommonVariableNames::argsSizes);
}

SgVariableDeclaration *
FortranCUDAModuleDeclarations::getDimensionsVariableDeclaration ()
{
  return variableDeclarations->get (CommonVariableNames::opDatDimensions);
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
      "Generating CUDA module scope declarations",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createDataSizesDeclaration ();

  createDimensionsDeclaration ();
}
