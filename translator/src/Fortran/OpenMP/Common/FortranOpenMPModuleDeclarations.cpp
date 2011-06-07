#include <FortranOpenMPModuleDeclarations.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranOpenMPModuleDeclarations::createOPDATDeclarations ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildPointerType;
  using SageInterface::appendStatement;
  using std::string;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      string const & variableName = VariableNames::getOpDatGlobalName (i);

      moduleDeclarations[variableName] = buildVariableDeclaration (
          variableName, buildPointerType (parallelLoop->get_OP_DAT_Type (i)),
          NULL, moduleScope);

      moduleDeclarations[variableName]->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (moduleDeclarations[variableName], moduleScope);
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

  moduleDeclarations[variableName] = buildVariableDeclaration (variableName,
      buildBoolType (), buildAssignInitializer (buildBoolValExp (true),
          buildBoolType ()), moduleScope);

  moduleDeclarations[variableName]->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (moduleDeclarations[variableName], moduleScope);
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
FortranOpenMPModuleDeclarations::getGlobalOPDATDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return moduleDeclarations[VariableNames::getOpDatGlobalName (
      OP_DAT_ArgumentGroup)];
}

SgVariableDeclaration *
FortranOpenMPModuleDeclarations::getFirstExecutionBooleanDeclaration ()
{
  return moduleDeclarations[getFirstExecutionBooleanVariableName ()];
}
