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

  for (map <string, SgType *>::const_iterator it = declarations->firstConstantDeclaration (); 
      it != declarations->lastConstantDeclaration (); 
      ++it)
  {
    string const variableName = it->first;

    SgExpression * moduleVariableExpr = constantVariableNames[variableName];

    SgExprStatement * assignmentStatement = buildAssignStatement (
        moduleVariableExpr,
        buildOpaqueVarRefExp (variableName, subroutineScope));

    appendStatement (assignmentStatement, subroutineScope);
  }
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


  for (map <string, SgType *>::const_iterator it = declarations->firstConstantDeclaration (); 
      it != declarations->lastConstantDeclaration (); 
      ++it)
  {
    string const variableName = it->first;

    SgType * type = it->second;
    
    SgExpression * moduleVariableExpr = buildDotExp(
        buildVarRefExp( variableDeclarations->get( OpenCL::CPP::globalConstants ) ),
        buildOpaqueVarRefExp( variableName ) );

    //string const moduleVariableName = "INTERNAL_" + variableName;

    constantVariableNames[variableName] = moduleVariableExpr;

    // not going to be a variable on its own
    /*SgVariableDeclaration * variableDeclaration =
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
            moduleVariableName, type, moduleScope);

    variableDeclarations->add (moduleVariableName, variableDeclaration); */
  }
}

std::map <std::string, std::string>::const_iterator
CPPInitialiseConstantsSubroutine::getFirstConstantName ()
{
  return constantVariableNames.begin ();
}

std::map <std::string, std::string>::const_iterator
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
