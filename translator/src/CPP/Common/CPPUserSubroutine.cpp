#include "CPPUserSubroutine.h"
#include "CPPParallelLoop.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "RoseHelper.h"

void
CPPUserSubroutine::analyseOpDeclConstReferences ()
{
  Debug::getInstance ()->debugMessage (
      "Analysing user subroutine for references to OP_DECL_CONSTs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  class TreeVisitor: public AstSimpleProcessing
  {
    private:

      CPPProgramDeclarationsAndDefinitions * declarations;

    public:

      TreeVisitor (CPPProgramDeclarationsAndDefinitions * declarations) :
        declarations (declarations)
      {
      }

      virtual void
      visit (SgNode * node)
      {
        using std::string;

        SgVarRefExp * variableReference = isSgVarRefExp (node);
        if (variableReference != NULL)
        {
          string const variableName =
              variableReference->get_symbol ()->get_name ();

          if (declarations->isOpConstDefinition (variableName))
          {
            Debug::getInstance ()->debugMessage (
                "Found reference to OP_DECL_CONST '" + variableName + "'",
                Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
          }
        }
      }
  };

  TreeVisitor * visitor = new TreeVisitor (declarations);

  visitor->traverse (subroutineHeaderStatement, preorder);
}

void
CPPUserSubroutine::createStatements ()
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
CPPUserSubroutine::createLocalVariableDeclarations ()
{
}

void
CPPUserSubroutine::createFormalParameterDeclarations ()
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

CPPUserSubroutine::CPPUserSubroutine (SgScopeStatement * moduleScope,
    CPPParallelLoop * parallelLoop,
    CPPProgramDeclarationsAndDefinitions * declarations) :
      UserSubroutine <SgFunctionDeclaration,
          CPPProgramDeclarationsAndDefinitions> (parallelLoop, declarations)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  originalSubroutine = declarations->getSubroutine (
      parallelLoop->getUserSubroutineName ());

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();

  analyseOpDeclConstReferences ();

  RoseHelper::forceOutputOfCodeToFile (subroutineHeaderStatement);
}
