#include <CPPModuleDeclarations.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPParallelLoop.h>
#include <ScopedVariableDeclarations.h>
#include <Debug.h>
#include <rose.h>

std::string const
CPPModuleDeclarations::getBlockSizeVariableName (
    std::string const & userSubroutineName)
{
  return "threadsPerBlockSize_" + userSubroutineName;
}

std::string const
CPPModuleDeclarations::getPartitionSizeVariableName (
    std::string const & userSubroutineName)
{
  return "setPartitionSize_" + userSubroutineName;
}

void
CPPModuleDeclarations::addParallelLoopSettingsVariables (
    SgScopeStatement * moduleScope,
    CPPProgramDeclarationsAndDefinitions * declarations)
{
  using namespace SageBuilder;
  using namespace SageInterface;
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
          variableName1, buildIntType (), buildAssignInitializer (buildIntVal (
              0), buildIntType ()), moduleScope);

      variableDeclarations ->add (variableName2, variableDeclaration2);

      appendStatement (variableDeclaration2, moduleScope);
    }
  }
}

SgVarRefExp *
CPPModuleDeclarations::getBlockSizeReference (
    std::string const & userSubroutineName)
{
  return variableDeclarations->getReference (getBlockSizeVariableName (
      userSubroutineName));
}

SgVarRefExp *
CPPModuleDeclarations::getPartitionSizeReference (
    std::string const & userSubroutineName)
{
  return variableDeclarations->getReference (getPartitionSizeVariableName (
      userSubroutineName));
}

CPPModuleDeclarations::CPPModuleDeclarations (SgScopeStatement * moduleScope,
    CPPProgramDeclarationsAndDefinitions * declarations)
{
  variableDeclarations = new ScopedVariableDeclarations ();

  addParallelLoopSettingsVariables (moduleScope, declarations);
}
