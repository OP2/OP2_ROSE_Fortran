#include <FortranModuleDeclarations.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranParallelLoop.h>
#include <SubroutineVariableDeclarations.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <rose.h>

/*
 * ======================================================
 * Private functions
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
          parallelLoop->getUserSubroutineName ());

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, buildBoolType (), buildAssignInitializer (buildBoolValExp (
          true), buildBoolType ()), moduleScope);

  variableDeclarations->add (variableName, variableDeclaration);

  variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration, moduleScope);
}

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

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

FortranModuleDeclarations::FortranModuleDeclarations (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  parallelLoop (parallelLoop), moduleScope (moduleScope)
{
  variableDeclarations = new SubroutineVariableDeclarations ();

  createFirstExecutionBooleanDeclaration ();

  if (parallelLoop->isDirectLoop () == false)
  {
    createCPlanReturnDeclaration ();
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgVariableDeclaration *
FortranModuleDeclarations::getCPlanReturnDeclaration ()
{
  return variableDeclarations->get (
      OP2::VariableNames::getPlanReturnVariableDeclarationName (
          parallelLoop->getUserSubroutineName ()));
}

SgVariableDeclaration *
FortranModuleDeclarations::getFirstExecutionBooleanDeclaration ()
{
  std::string const & variableName =
      OP2::VariableNames::getFirstTimeExecutionVariableDeclarationName (
          parallelLoop->getUserSubroutineName ());

  return variableDeclarations->get (variableName);
}

SgVariableDeclaration *
FortranModuleDeclarations::getReductionArrayHostDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (
      OP2::VariableNames::getReductionArrayHostName (OP_DAT_ArgumentGroup,
          parallelLoop->getUserSubroutineName ()));
}

SgVariableDeclaration *
FortranModuleDeclarations::getReductionArrayDeviceDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (
      OP2::VariableNames::getReductionArrayDeviceName (OP_DAT_ArgumentGroup,
          parallelLoop->getUserSubroutineName ()));
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
