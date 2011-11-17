#include <FortranModuleDeclarations.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranParallelLoop.h>
#include <ScopedVariableDeclarations.h>
#include <Debug.h>
#include <CompilerGeneratedNames.h>
#include <rose.h>

ScopedVariableDeclarations *
FortranModuleDeclarations::getDeclarations ()
{
  return variableDeclarations;
}

void
FortranModuleDeclarations::createCPlanReturnDeclaration ()
{
  using namespace OP2VariableNames;
  using std::string;

  SgType * c_ptrType = FortranTypesBuilder::buildClassDeclaration ("c_ptr",
      moduleScope)->get_type ();

  std::string const & variableName = getPlanReturnVariableDeclarationName (
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
