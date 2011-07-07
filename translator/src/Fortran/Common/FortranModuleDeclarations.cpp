#include <FortranModuleDeclarations.h>
#include <Debug.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

void
FortranModuleDeclarations::createFirstExecutionBooleanDeclaration ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildBoolType;
  using SageBuilder::buildBoolValExp;
  using SageBuilder::buildAssignInitializer;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating first time execution boolean at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  string const & variableName = getFirstExecutionBooleanVariableName ();

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, buildBoolType (), buildAssignInitializer (buildBoolValExp (
          true), buildBoolType ()), moduleScope);

  moduleDeclarations->add (variableName, variableDeclaration);

  variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration, moduleScope);
}

std::string
FortranModuleDeclarations::getFirstExecutionBooleanVariableName ()
{
  return "isFirstTime_" + userSubroutineName;
}

FortranModuleDeclarations::FortranModuleDeclarations (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  userSubroutineName (userSubroutineName), parallelLoop (parallelLoop),
      moduleScope (moduleScope)
{
  moduleDeclarations = new SubroutineVariableDeclarations ();

  createFirstExecutionBooleanDeclaration ();
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgVariableDeclaration *
FortranModuleDeclarations::getFirstExecutionBooleanDeclaration ()
{
  return moduleDeclarations->get (getFirstExecutionBooleanVariableName ());
}

SgVariableDeclaration *
FortranModuleDeclarations::getGlobalOpDatDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return moduleDeclarations->get (VariableNames::getOpDatGlobalName (
      OP_DAT_ArgumentGroup));
}
