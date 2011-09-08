#include <FortranInitialiseConstantsSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <RoseHelper.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranInitialiseConstantsSubroutine::createStatements ()
{
}

void
FortranInitialiseConstantsSubroutine::createLocalVariableDeclarations ()
{
}

void
FortranInitialiseConstantsSubroutine::createFormalParameterDeclarations ()
{
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
FortranInitialiseConstantsSubroutine::generateSubroutine ()
{
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createStatements ();
}

void
FortranInitialiseConstantsSubroutine::declareConstants ()
{
  using std::map;
  using std::string;

  for (map <string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    string const variableName = it->first;

    string const moduleVariableName = "INTERNAL_" + variableName;

    constantVariableNames[variableName] = moduleVariableName;

    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            moduleVariableName, declarations->getType (variableName),
            moduleScope, 1, CONSTANT);

    variableDeclarations->add (moduleVariableName, variableDeclaration);
  }
}

std::map <std::string, std::string>::const_iterator
FortranInitialiseConstantsSubroutine::getFirstConstantName ()
{
  return constantVariableNames.begin ();
}

std::map <std::string, std::string>::const_iterator
FortranInitialiseConstantsSubroutine::getLastConstantName ()
{
  return constantVariableNames.end ();
}

FortranInitialiseConstantsSubroutine::FortranInitialiseConstantsSubroutine (
    SgScopeStatement * moduleScope,
    FortranProgramDeclarationsAndDefinitions * declarations) :
  Subroutine <SgProcedureHeaderStatement> ("InitialiseConstants"), moduleScope (
      moduleScope), declarations (declarations)
{
}
