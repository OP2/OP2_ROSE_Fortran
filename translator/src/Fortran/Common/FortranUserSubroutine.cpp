#include "FortranUserSubroutine.h"
#include "FortranParallelLoop.h"
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "RoseStatementsAndExpressionsBuilder.h"

void
FortranUserSubroutine::createStatements ()
{
  using namespace SageInterface;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Outputting and modifying statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <SgStatement *> originalStatements =
      originalSubroutine->get_definition ()->get_body ()->get_statements ();

  for (vector <SgStatement *>::iterator it = originalStatements.begin (); it
      != originalStatements.end (); ++it)
  {
    appendStatement (*it, subroutineScope);
  }
}

void
FortranUserSubroutine::createLocalVariableDeclarations ()
{

}

void
FortranUserSubroutine::createFormalParameterDeclarations ()
{
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Outputting formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgFunctionParameterList * originalParameters =
      originalSubroutine->get_parameterList ();

  for (SgInitializedNamePtrList::iterator paramIt =
      originalParameters->get_args ().begin (); paramIt
      != originalParameters->get_args ().end (); ++paramIt)
  {
    string const variableName = (*paramIt)->get_name ().getString ();

    SgType * type = (*paramIt)->get_type ();

    SgVariableDeclaration
        * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, type, subroutineScope, formalParameters);
  }
}

FortranUserSubroutine::FortranUserSubroutine (SgScopeStatement * moduleScope,
    FortranParallelLoop * parallelLoop,
    FortranProgramDeclarationsAndDefinitions * declarations) :
  UserSubroutine <SgProcedureHeaderStatement,
      FortranProgramDeclarationsAndDefinitions> (parallelLoop, declarations)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  originalSubroutine = declarations->getSubroutine (
      parallelLoop->getUserSubroutineName ());
}
