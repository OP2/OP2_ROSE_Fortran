#include <FortranOpenMPModuleDeclarations.h>
#include <FortranParallelLoop.h>
#include <ScopedVariableDeclarations.h>
#include <CompilerGeneratedNames.h>
#include <Debug.h>
#include <rose.h>

void
FortranOpenMPModuleDeclarations::createOpDatDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT declarations at module scope", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = getOpDatGlobalName (i);

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName, buildPointerType (parallelLoop->getOpDatType (i)),
          NULL, moduleScope);

      variableDeclarations->add (variableName, variableDeclaration);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variableDeclaration, moduleScope);
    }
  }
}

FortranOpenMPModuleDeclarations::FortranOpenMPModuleDeclarations (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranModuleDeclarations (parallelLoop, moduleScope)
{
}
