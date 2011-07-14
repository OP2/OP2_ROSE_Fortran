#include <FortranCUDAModuleDeclarations.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

void
FortranCUDAModuleDeclarations::createOpDatSizesDeclarations ()
{
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT size declarations at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = VariableNames::getOpDatSizeName (i);

      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), moduleScope);
    }
  }
}

FortranCUDAModuleDeclarations::FortranCUDAModuleDeclarations (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranModuleDeclarations (userSubroutineName, parallelLoop, moduleScope)
{
  createOpDatSizesDeclarations ();
}
