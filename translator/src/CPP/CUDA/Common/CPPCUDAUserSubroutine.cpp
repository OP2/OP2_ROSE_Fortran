#include <CPPCUDAUserSubroutine.h>
#include <CPPParallelLoop.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <Exceptions.h>

void
CPPCUDAUserSubroutine::forceOutputOfCodeToFile ()
{
  Debug::getInstance ()->debugMessage (
      "Ensuring user subroutine is generated in output file",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * We have to set each node in the AST representation of
   * this subroutine as compiler generated, otherwise chunks
   * of the user kernel are missing in the output
   * ======================================================
   */

  class TreeVisitor: public AstSimpleProcessing
  {
    public:

      TreeVisitor ()
      {
      }

      virtual void
      visit (SgNode * node)
      {
        SgLocatedNode * locatedNode = isSgLocatedNode (node);
        if (locatedNode != NULL)
        {
          locatedNode->setOutputInCodeGeneration ();
        }
      }
  };

  TreeVisitor * visitor = new TreeVisitor ();

  visitor->traverse (subroutineHeaderStatement, preorder);
}

void
CPPCUDAUserSubroutine::findOriginalSubroutine ()
{
  using boost::iequals;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Searching for original user subroutine '"
          + parallelLoop->getUserSubroutineName () + "'",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (vector <SgFunctionDeclaration *>::const_iterator it =
      declarations->firstSubroutineInSourceCode (); it
      != declarations->lastSubroutineInSourceCode (); ++it)
  {
    SgFunctionDeclaration * subroutine = *it;

    if (iequals (parallelLoop->getUserSubroutineName (),
        subroutine->get_name ().getString ()))
    {
      Debug::getInstance ()->debugMessage ("Found user kernel '"
          + parallelLoop->getUserSubroutineName () + "'",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      originalSubroutine = subroutine;
      break;
    }
  }

  if (originalSubroutine == NULL)
  {
    throw Exceptions::CodeGeneration::UnknownSubroutineException (
        "Unable to find user kernel '" + parallelLoop->getUserSubroutineName ()
            + "'");
  }
}

void
CPPCUDAUserSubroutine::createStatements ()
{
  using boost::iequals;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Outputting and modifying statements",
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

  vector <SgStatement *> originalStatements =
      originalSubroutine->get_definition ()->get_body ()->get_statements ();

  for (vector <SgStatement *>::iterator it = originalStatements.begin (); it
      != originalStatements.end (); ++it)
  {
    appendStatement (*it, subroutineScope);
  }
}

void
CPPCUDAUserSubroutine::createLocalVariableDeclarations ()
{
}

void
CPPCUDAUserSubroutine::createFormalParameterDeclarations ()
{
}

CPPCUDAUserSubroutine::CPPCUDAUserSubroutine (SgScopeStatement * moduleScope,
    CPPParallelLoop * parallelLoop,
    CPPProgramDeclarationsAndDefinitions * declarations) :
      UserSubroutine <SgFunctionDeclaration,
          CPPProgramDeclarationsAndDefinitions> (parallelLoop, declarations)
{
  using SageBuilder::buildDefiningFunctionDeclaration;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  subroutineHeaderStatement->get_functionModifier ().setCudaDevice ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  findOriginalSubroutine ();

  createStatements ();

  forceOutputOfCodeToFile ();
}
