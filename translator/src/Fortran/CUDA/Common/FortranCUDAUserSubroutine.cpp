#include <boost/algorithm/string/predicate.hpp>
#include <FortranCUDAUserSubroutine.h>
#include <Debug.h>
#include <algorithm>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranCUDAUserSubroutine::forceOutputOfCodeToFile ()
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
FortranCUDAUserSubroutine::findOriginalSubroutine ()
{
  using boost::iequals;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Searching for original user subroutine '" + hostSubroutineName + "'",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (vector <SgProcedureHeaderStatement *>::const_iterator it =
      declarations->firstSubroutineInSourceCode (); it
      != declarations->lastSubroutineInSourceCode (); ++it)
  {
    SgProcedureHeaderStatement * subroutine = *it;

    if (iequals (hostSubroutineName, subroutine->get_name ().getString ()))
    {
      Debug::getInstance ()->debugMessage ("Found user kernel '"
          + hostSubroutineName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__,
          __LINE__);

      originalSubroutine = subroutine;
      break;
    }
  }

  if (originalSubroutine == NULL)
  {
    Debug::getInstance ()->errorMessage ("Unable to find user kernel '"
        + hostSubroutineName + "'", __FILE__, __LINE__);
  }
}

void
FortranCUDAUserSubroutine::createStatements ()
{
  using boost::iequals;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Outputting and modifying statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgFunctionParameterList * originalParameters =
      originalSubroutine->get_parameterList ();

  vector <SgStatement *> originalStatements =
      originalSubroutine->get_definition ()->get_body ()->get_statements ();

  for (vector <SgStatement *>::iterator it = originalStatements.begin (); it
      != originalStatements.end (); ++it)
  {
    SgVariableDeclaration * isVariableDeclaration = isSgVariableDeclaration (
        *it);

    if (isVariableDeclaration == NULL)
    {
      Debug::getInstance ()->debugMessage (
          "Appending (non-variable-declaration) statement",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      appendStatement (*it, subroutineScope);
    }
    else
    {
      Debug::getInstance ()->debugMessage ("Appending variable declaration",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      unsigned int OP_DAT_ArgumentGroup = 1;

      for (SgInitializedNamePtrList::iterator variableIt =
          isVariableDeclaration->get_variables ().begin (); variableIt
          != isVariableDeclaration->get_variables ().end (); ++variableIt)
      {
        string const variableName = (*variableIt)->get_name ().getString ();

        SgType * type = (*variableIt)->get_typeptr ();

        bool isFormalParamater = false;

        for (SgInitializedNamePtrList::iterator paramIt =
            originalParameters->get_args ().begin (); paramIt
            != originalParameters->get_args ().end (); ++paramIt)
        {
          string const formalParamterName =
              (*paramIt)->get_name ().getString ();

          if (iequals (variableName, formalParamterName))
          {
            Debug::getInstance ()->debugMessage ("'" + variableName
                + "' is a formal parameter", Debug::HIGHEST_DEBUG_LEVEL,
                __FILE__, __LINE__);

            isFormalParamater = true;

            if (parallelLoop->isIndirect (OP_DAT_ArgumentGroup)
                && parallelLoop->isRead (OP_DAT_ArgumentGroup))
            {
              SgVariableDeclaration
                  * variableDeclaration =
                      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, type, subroutineScope,
                          formalParameters, 2, DEVICE, SHARED);
            }
            else if (parallelLoop->isGlobalScalar (OP_DAT_ArgumentGroup)
                && parallelLoop->isRead (OP_DAT_ArgumentGroup))
            {
              SgVariableDeclaration
                  * variableDeclaration =
                      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, type, subroutineScope,
                          formalParameters, 1, VALUE);
            }
            else
            {
              SgVariableDeclaration
                  * variableDeclaration =
                      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, type, subroutineScope,
                          formalParameters, 1, DEVICE);
            }

            ++OP_DAT_ArgumentGroup;
          }
        }

        if (isFormalParamater == false)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
              + "' is NOT a formal parameter", Debug::HIGHEST_DEBUG_LEVEL,
              __FILE__, __LINE__);

          SgVariableDeclaration
              * variableDeclaration =
                  FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                      variableName, type, subroutineScope, 1, DEVICE);
        }
      }
    }
  }
}

void
FortranCUDAUserSubroutine::createLocalVariableDeclarations ()
{
}

void
FortranCUDAUserSubroutine::createFormalParameterDeclarations ()
{
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranCUDAUserSubroutine::FortranCUDAUserSubroutine (
    std::string const & subroutineName, SgScopeStatement * moduleScope,
    FortranProgramDeclarationsAndDefinitions * declarations,
    FortranParallelLoop * parallelLoop) :
  UserSubroutine <SgProcedureHeaderStatement,
      FortranProgramDeclarationsAndDefinitions> (subroutineName, declarations,
      parallelLoop)
{
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(device) ",
      AstUnparseAttribute::e_before);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  findOriginalSubroutine ();

  createStatements ();

  forceOutputOfCodeToFile ();
}
