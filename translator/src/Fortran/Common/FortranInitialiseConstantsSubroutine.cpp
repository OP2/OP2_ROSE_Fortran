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
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  using std::string;
  using std::map;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating statements in initialise constants subroutine",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (map <string, SgType *>::const_iterator it =
      declarations->firstConstantDeclaration (); it
      != declarations->lastConstantDeclaration (); ++it)
  {
    string const variableName = it->first;

    string const moduleVariableName = constantVariableNames[variableName];

    SgExprStatement * assignmentStatement = buildAssignStatement (
        buildVarRefExp (variableDeclarations->get (moduleVariableName)),
        buildOpaqueVarRefExp (variableName, subroutineScope));

    appendStatement (assignmentStatement, subroutineScope);
  }
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

  for (map <string, SgType *>::const_iterator it =
      declarations->firstConstantDeclaration (); it
      != declarations->lastConstantDeclaration (); ++it)
  {
    string const variableName = it->first;

    SgType * type = it->second;

    string const moduleVariableName = "INTERNAL_" + variableName;

    constantVariableNames[variableName] = moduleVariableName;

    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            moduleVariableName, type, moduleScope);

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
