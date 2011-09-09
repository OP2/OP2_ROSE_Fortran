#include <FortranCUDAModuleDeclarationsIndirectLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

void
FortranCUDAModuleDeclarationsIndirectLoop::createCPlanDeclaration ()
{
  SgType * c_ptrType = FortranTypesBuilder::buildClassDeclaration ("c_ptr",
      moduleScope)->get_type ();

  variableDeclarations->add (OP2::VariableNames::PlanFunction::planRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::planRet, c_ptrType, moduleScope));
}

SgVariableDeclaration *
FortranCUDAModuleDeclarationsIndirectLoop::getCPlanDeclaration ()
{
  return variableDeclarations->get (OP2::VariableNames::PlanFunction::planRet);
}

FortranCUDAModuleDeclarationsIndirectLoop::FortranCUDAModuleDeclarationsIndirectLoop (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration) :
  FortranCUDAModuleDeclarations (userSubroutineName, parallelLoop, moduleScope,
      dataSizesDeclaration, dimensionsDeclaration)
{
  createCPlanDeclaration ();
}
