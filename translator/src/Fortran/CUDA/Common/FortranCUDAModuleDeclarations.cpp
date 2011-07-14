#include <FortranCUDAModuleDeclarations.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

void
FortranCUDAModuleDeclarations::createDataSizesDeclaration ()
{
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT size declarations at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  string const & variableName = CommonVariableNames::argsSizes + "Global";

  SgVariableDeclaration * variableDeclaration =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName, dataSizesDeclaration->getType (), moduleScope);

  moduleDeclarations->add (variableName, variableDeclaration);
}

FortranCUDAModuleDeclarations::FortranCUDAModuleDeclarations (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranCUDADataSizesDeclaration * dataSizesDeclaration) :
  FortranModuleDeclarations (userSubroutineName, parallelLoop, moduleScope),
      dataSizesDeclaration (dataSizesDeclaration)
{
  createDataSizesDeclaration ();
}

FortranCUDADataSizesDeclaration *
FortranCUDAModuleDeclarations::getDataSizesDeclaration ()
{
  return dataSizesDeclaration;
}
