#include "FortranCUDAModuleDeclarations.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "FortranParallelLoop.h"
#include "ScopedVariableDeclarations.h"
#include "PlanFunctionNames.h"
#include <rose.h>

FortranCUDAModuleDeclarations::FortranCUDAModuleDeclarations (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration) :
  FortranModuleDeclarations (parallelLoop, moduleScope), dataSizesDeclaration (
      dataSizesDeclaration), dimensionsDeclaration (dimensionsDeclaration)
{
  using namespace PlanFunctionVariableNames;
  using std::string;

  if (parallelLoop->isDirectLoop () == false)
  {
    string const & variableName = getPlanReturnVariableName (
        parallelLoop->getUserSubroutineName ());

    variableDeclarations->add (variableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, FortranTypesBuilder::buildClassDeclaration ("c_ptr",
                moduleScope)->get_type (), moduleScope));
  }
}
