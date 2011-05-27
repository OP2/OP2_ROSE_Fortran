#include <OpenMPModuleDeclarations.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
OpenMPModuleDeclarations::createFirstExecutionBooleanDeclaration (
    SgScopeStatement * moduleScope)
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildBoolType;
  using std::string;

  string const & variableName = getFirstExecutionBooleanVariableName ();

  moduleDeclarations[variableName] = buildVariableDeclaration (variableName,
      buildBoolType (), NULL, moduleScope);
}

std::string
OpenMPModuleDeclarations::getFirstExecutionBooleanVariableName ()
{
  return "isFirstTime_" + userSubroutineName;
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgVarRefExp *
OpenMPModuleDeclarations::getReferenceToFirstExecutionBoolean ()
{
  using SageBuilder::buildVarRefExp;

  return buildVarRefExp (
      moduleDeclarations[getFirstExecutionBooleanVariableName ()]);
}

OpenMPModuleDeclarations::OpenMPModuleDeclarations (
    SgScopeStatement * moduleScope, std::string const & userSubroutineName)
{

}
