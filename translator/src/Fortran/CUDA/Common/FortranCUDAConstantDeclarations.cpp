#include "FortranCUDAConstantDeclarations.h"
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "OP2Definitions.h"
#include "ScopedVariableDeclarations.h"
#include "Debug.h"
#include <rose.h>

std::string
FortranCUDAConstantDeclarations::getNewConstantVariableName (
    std::string const & originalName)
{
  return originalName + "_CUDA";
}

void
FortranCUDAConstantDeclarations::addDeclarations (
    FortranProgramDeclarationsAndDefinitions * declarations,
    SgScopeStatement * moduleScope)
{
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Adding variables with constant access specifiers to module",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (map <string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    string const & variableName = it->first;

    OpConstDefinition * constDefinition = it->second;

    SgType * type = constDefinition->getType ();

    Debug::getInstance ()->debugMessage ("Analysing OP_DECL_CONST with name '"
        + variableName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    string const & newVariableName = getNewConstantVariableName (variableName);

    oldNamesToNewNames[variableName] = newVariableName;

    variableDeclarations->add (newVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            newVariableName, type, moduleScope, 1, CUDA_CONSTANT));
  }
}

SgVarRefExp *
FortranCUDAConstantDeclarations::getReferenceToNewVariable (
    std::string const & originalName)
{
  return variableDeclarations->getReference (getNewConstantVariableName (
      originalName));
}

bool
FortranCUDAConstantDeclarations::isCUDAConstant (
    std::string const & originalName)
{
  return oldNamesToNewNames.count (originalName) != 0;
}

FortranCUDAConstantDeclarations::FortranCUDAConstantDeclarations (
    FortranProgramDeclarationsAndDefinitions * declarations,
    SgScopeStatement * moduleScope)
{
  variableDeclarations = new ScopedVariableDeclarations ();

  addDeclarations (declarations, moduleScope);
}
