#include <FortranCUDAModuleDeclarationsIndirectLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranPlan.h>

void
FortranCUDAModuleDeclarationsIndirectLoop::createCPlanDeclaration ()
{
  SgType * c_ptrType = FortranTypesBuilder::buildClassDeclaration ("c_ptr",
      moduleScope)->get_type ();

  variableDeclarations->add (PlanFunction::Fortran::planRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::Fortran::planRet, c_ptrType, moduleScope));
}

SgVariableDeclaration *
FortranCUDAModuleDeclarationsIndirectLoop::getCPlanDeclaration ()
{
  return variableDeclarations->get (PlanFunction::Fortran::planRet);
}

FortranCUDAModuleDeclarationsIndirectLoop::FortranCUDAModuleDeclarationsIndirectLoop (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration) :
  FortranCUDAModuleDeclarations (userSubroutineName, parallelLoop, moduleScope,
      dataSizesDeclaration, dimensionsDeclaration)
{
  createCPlanDeclaration();
}
