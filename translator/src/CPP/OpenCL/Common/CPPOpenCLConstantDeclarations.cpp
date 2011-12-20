#include "CPPOpenCLConstantDeclarations.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "ScopedVariableDeclarations.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "OP2Definitions.h"
#include <rose.h>

void
CPPOpenCLConstantDeclarations::addDeclarations (
    CPPProgramDeclarationsAndDefinitions * declarations,
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

    SgVariableDeclaration * variableDeclaration =
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, type, moduleScope);

    variableDeclaration->get_declarationModifier ().get_storageModifier ().setExtern ();

    variableDeclarations->add (variableName, variableDeclaration);
  }
}

ScopedVariableDeclarations *
CPPOpenCLConstantDeclarations::getDeclarations ()
{
  return variableDeclarations;
}

CPPOpenCLConstantDeclarations::CPPOpenCLConstantDeclarations (
    CPPProgramDeclarationsAndDefinitions * declarations,
    SgScopeStatement * moduleScope)
{
  variableDeclarations = new ScopedVariableDeclarations ();

  addDeclarations (declarations, moduleScope);
}
