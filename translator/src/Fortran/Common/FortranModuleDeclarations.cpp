#include <FortranModuleDeclarations.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranParallelLoop.h>
#include <ScopedVariableDeclarations.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <rose.h>

void
FortranModuleDeclarations::createCPlanReturnDeclaration ()
{
  using std::string;

  SgType * c_ptrType = FortranTypesBuilder::buildClassDeclaration ("c_ptr",
      moduleScope)->get_type ();

  std::string const & variableName =
      OP2::VariableNames::getPlanReturnVariableDeclarationName (
          parallelLoop->getUserSubroutineName ());

  variableDeclarations->add (variableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName, c_ptrType, moduleScope));
}

FortranModuleDeclarations::FortranModuleDeclarations (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  parallelLoop (parallelLoop), moduleScope (moduleScope)
{
  variableDeclarations = new ScopedVariableDeclarations ();

  if (parallelLoop->isDirectLoop () == false)
  {
    createCPlanReturnDeclaration ();
  }
}

SgVariableDeclaration *
FortranModuleDeclarations::getCPlanReturnDeclaration ()
{
  return variableDeclarations->get (
      OP2::VariableNames::getPlanReturnVariableDeclarationName (
          parallelLoop->getUserSubroutineName ()));
}

SgVariableDeclaration *
FortranModuleDeclarations::getGlobalOpDatDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (OP2::VariableNames::getOpDatGlobalName (
      OP_DAT_ArgumentGroup));
}
