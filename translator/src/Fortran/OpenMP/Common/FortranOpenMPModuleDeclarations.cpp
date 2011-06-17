#include <FortranOpenMPModuleDeclarations.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranOpenMPModuleDeclarations::createOpDatDeclarations ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildPointerType;
  using SageInterface::appendStatement;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = VariableNames::getOpDatGlobalName (i);

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName, buildPointerType (parallelLoop->getOpDatType (i)),
          NULL, moduleScope);

      moduleDeclarations->add (variableName, variableDeclaration);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variableDeclaration, moduleScope);
    }
  }
}

void
FortranOpenMPModuleDeclarations::createFirstExecutionBooleanDeclaration ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildBoolType;
  using SageBuilder::buildBoolValExp;
  using SageBuilder::buildAssignInitializer;
  using SageInterface::appendStatement;
  using std::string;

  string const & variableName = getFirstExecutionBooleanVariableName ();

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, buildBoolType (), buildAssignInitializer (buildBoolValExp (
          true), buildBoolType ()), moduleScope);

  moduleDeclarations->add (variableName, variableDeclaration);

  variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration, moduleScope);
}

std::string
FortranOpenMPModuleDeclarations::getFirstExecutionBooleanVariableName ()
{
  return "isFirstTime_" + userSubroutineName;
}

FortranOpenMPModuleDeclarations::FortranOpenMPModuleDeclarations (
    std::string const & userSubroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope)
{
  this->userSubroutineName = userSubroutineName;

  this->parallelLoop = parallelLoop;

  this->moduleScope = moduleScope;

  createFirstExecutionBooleanDeclaration ();
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgVariableDeclaration *
FortranOpenMPModuleDeclarations::getGlobalOpDatDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return moduleDeclarations->get (VariableNames::getOpDatGlobalName (
      OP_DAT_ArgumentGroup));
}

SgVariableDeclaration *
FortranOpenMPModuleDeclarations::getFirstExecutionBooleanDeclaration ()
{
  return moduleDeclarations->get (getFirstExecutionBooleanVariableName ());
}
