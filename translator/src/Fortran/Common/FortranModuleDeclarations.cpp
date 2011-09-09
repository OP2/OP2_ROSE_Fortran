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

  std::string const & variableName =
      OP2::VariableNames::getFirstTimeExecutionVariableDeclarationName (
          userSubroutineName);

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, buildBoolType (), buildAssignInitializer (buildBoolValExp (
          true), buildBoolType ()), moduleScope);

  variableDeclarations->add (variableName, variableDeclaration);

  variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration, moduleScope);
}

FortranModuleDeclarations::FortranModuleDeclarations (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  userSubroutineName (userSubroutineName), parallelLoop (parallelLoop),
      moduleScope (moduleScope)
{
  variableDeclarations = new SubroutineVariableDeclarations ();

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
  std::string const & variableName =
      OP2::VariableNames::getFirstTimeExecutionVariableDeclarationName (
          userSubroutineName);

  return variableDeclarations->get (variableName);
}

SgVariableDeclaration *
FortranModuleDeclarations::getReductionArrayHostDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (
      OP2::VariableNames::getReductionArrayHostName (OP_DAT_ArgumentGroup,
          userSubroutineName));
}

SgVariableDeclaration *
FortranModuleDeclarations::getReductionArrayDeviceDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (
      OP2::VariableNames::getReductionArrayDeviceName (OP_DAT_ArgumentGroup,
          userSubroutineName));
}

SgVariableDeclaration *
FortranModuleDeclarations::getGlobalOpDatDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (OP2::VariableNames::getOpDatGlobalName (
      OP_DAT_ArgumentGroup));
}

SubroutineVariableDeclarations *
FortranModuleDeclarations::getAllDeclarations ()
{
  return variableDeclarations;
}
