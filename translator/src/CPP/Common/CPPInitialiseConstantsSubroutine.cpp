#include <CPPInitialiseConstantsSubroutine.h>
//#include <CPPStatementsAndExpressionsBuilder.h>
//#include <CPPTypesBuilder.h>
#include <RoseHelper.h>
#include <CommonNamespaces.h>
using namespace SageBuilder;

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPInitialiseConstantsSubroutine::createStatements ()
{
}

void
CPPInitialiseConstantsSubroutine::createLocalVariableDeclarations ()
{
}

void
CPPInitialiseConstantsSubroutine::createFormalParameterDeclarations ()
{
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
CPPInitialiseConstantsSubroutine::generateSubroutine ()
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
CPPInitialiseConstantsSubroutine::declareConstants ()
{
  using std::map;
  using std::string;

  for (map <string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    string const variableName = it->first;

    SgExpression * moduleVariableExpr = buildDotExp (buildVarRefExp (
        variableDeclarations->get (OpenCL::CPP::globalConstants)),
        buildOpaqueVarRefExp (variableName));

    constantVariableNames[variableName] = moduleVariableExpr;
  }
}

std::map <std::string, SgExpression *>::const_iterator
CPPInitialiseConstantsSubroutine::getFirstConstantName ()
{
  return constantVariableNames.begin ();
}

std::map <std::string, SgExpression *>::const_iterator
CPPInitialiseConstantsSubroutine::getLastConstantName ()
{
  return constantVariableNames.end ();
}

CPPInitialiseConstantsSubroutine::CPPInitialiseConstantsSubroutine (
    SgScopeStatement * moduleScope,
    CPPProgramDeclarationsAndDefinitions * declarations) :
  Subroutine <SgProcedureHeaderStatement> ("InitialiseConstants"), moduleScope (
      moduleScope), declarations (declarations)
{
}
