#include "CPPModuleDeclarations.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPParallelLoop.h"
#include "ScopedVariableDeclarations.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include <rose.h>

void
CPPModuleDeclarations::addParallelLoopSettingsVariables (
    SgScopeStatement * moduleScope,
    CPPProgramDeclarationsAndDefinitions * declarations)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Adding variables to file to control OP2 block size and partition size settings",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    CPPParallelLoop * parallelLoop =
        static_cast <CPPParallelLoop *> (it->second);

    std::string const & variableName1 = getBlockSizeVariableName (
        userSubroutineName);

    SgVariableDeclaration * variableDeclaration1 = buildVariableDeclaration (
        variableName1, buildIntType (), buildAssignInitializer (buildIntVal (
            512), buildIntType ()), moduleScope);

    variableDeclarations ->add (variableName1, variableDeclaration1);

    appendStatement (variableDeclaration1, moduleScope);

    if (parallelLoop->isDirectLoop () == false)
    {
      std::string const & variableName2 = getPartitionSizeVariableName (
          userSubroutineName);

      SgVariableDeclaration * variableDeclaration2 = buildVariableDeclaration (
          variableName2, buildIntType (), buildAssignInitializer (buildIntVal (
              0), buildIntType ()), moduleScope);

      variableDeclarations ->add (variableName2, variableDeclaration2);

      appendStatement (variableDeclaration2, moduleScope);
    }
  }
}

ScopedVariableDeclarations *
CPPModuleDeclarations::getDeclarations ()
{
  return variableDeclarations;
}

CPPModuleDeclarations::CPPModuleDeclarations (SgScopeStatement * moduleScope,
    CPPProgramDeclarationsAndDefinitions * declarations)
{
  variableDeclarations = new ScopedVariableDeclarations ();

  addParallelLoopSettingsVariables (moduleScope, declarations);
}
