#include <boost/algorithm/string/predicate.hpp>
#include <UserDeviceSubroutine.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
UserDeviceSubroutine::copyAndModifySubroutine (SgScopeStatement * moduleScope,
    Declarations & declarations)
{
  using boost::iequals;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildFunctionParameterList;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Modifying and outputting original kernel to CUDA file", 2);

  SgProcedureHeaderStatement * originalSubroutine = NULL;
  SgFunctionParameterList * originalParameters = NULL;

  /*
   * ======================================================
   * Find the original subroutine (otherwise the definition
   * would be empty)
   * ======================================================
   */

  for (vector <SgProcedureHeaderStatement *>::const_iterator it =
      declarations.first_SubroutineInSourceCode (); it
      != declarations.last_SubroutineInSourceCode (); ++it)
  {
    SgProcedureHeaderStatement * subroutine = *it;

    if (iequals (userHostSubroutineName, subroutine->get_name ().getString ()))
    {
      /*
       * ======================================================
       * Grab the subroutine and its parameters
       * ======================================================
       */
      originalSubroutine = subroutine;
      originalParameters = subroutine->get_parameterList ();
      break;
    }
  }

  ROSE_ASSERT (originalSubroutine != NULL);

  /*
   * ======================================================
   * Create the new subroutine
   * ======================================================
   */
  formalParameters = buildFunctionParameterList ();

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  /*
   * ======================================================
   * Currently have to add the 'attribute' statement by hand
   * as there is no grammar support
   * ======================================================
   */
  addTextForUnparser (subroutineHeaderStatement, "attributes(device) ",
      AstUnparseAttribute::e_before);

  /*
   * ======================================================
   * Visit all the statements in the original subroutine
   * and append them to new subroutine while making necessary
   * changes.
   * TODO: This may not work for nested bodies. The alternative
   * is to use a copy routine, but currently not working either
   * ======================================================
   */
  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  vector <SgStatement *> originalStatements =
      originalSubroutine->get_definition ()->get_body ()->get_statements ();

  for (vector <SgStatement *>::iterator it = originalStatements.begin (); it
      != originalStatements.end (); ++it)
  {
    /*
     * ======================================================
     * Copy the statement if:
     * 1) It is NOT a variable declaration
     * 2) It is a variable declaration BUT it is not a formal
     * parameter declaration
     * ======================================================
     */
    SgVariableDeclaration * isVariableDeclaration = isSgVariableDeclaration (
        *it);

    if (isVariableDeclaration == NULL)
    {
      appendStatement (*it, subroutineScope);

      if (isSgImplicitStatement (*it) != NULL)
      {
        /*
         * ======================================================
         * Append the variable declarations immediately after the
         * 'IMPLICIT NONE' statement
         * ======================================================
         */

        for (SgInitializedNamePtrList::iterator paramIt =
            originalParameters->get_args ().begin (); paramIt
            != originalParameters->get_args ().end (); ++paramIt)
        {
          /*
           * ======================================================
           * Build a new variable declaration using the properties
           * of the original declaration
           * ======================================================
           */
          SgVariableDeclaration * newVariableDeclaration =
              buildVariableDeclaration ((*paramIt)->get_name ().getString (),
                  (*paramIt)->get_typeptr (), NULL, subroutineScope);

          /*
           * ======================================================
           * Set the Fortran attributes of the declared variables
           * ======================================================
           */
          newVariableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
          newVariableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();

          formalParameters->append_arg (
              *(newVariableDeclaration->get_variables ().begin ()));

          /*
           * ======================================================
           * Append the variable declaration to the body of the new
           * subroutine
           * ======================================================
           */
          appendStatement (newVariableDeclaration, subroutineScope);
        }
      }
    }
    else
    {
      bool isFormalParameter = false;

      string const
          variableName =
              isVariableDeclaration->get_definition ()->get_vardefn ()->get_name ().getString ();

      for (SgInitializedNamePtrList::iterator paramIt =
          formalParameters->get_args ().begin (); paramIt
          != formalParameters->get_args ().end (); ++paramIt)
      {
        string const parameterName = (*paramIt)->get_name ().getString ();

        if (iequals (variableName, parameterName))
        {
          isFormalParameter = true;
        }
      }

      if (isFormalParameter == false)
      {
        /*
         * ======================================================
         * Append the statement to the new subroutine only if it
         * is not a formal parameter
         * ======================================================
         */
        appendStatement (*it, subroutineScope);
      }
    }
  }

  /*
   * ======================================================
   * The following class visits every created node for the
   * subroutine and ensures that it is generated during
   * unparsing
   * ======================================================
   */
  class GenerateAllNodesInUnparser: public AstSimpleProcessing
  {
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
  (new GenerateAllNodesInUnparser ())->traverse (subroutineHeaderStatement,
      preorder);
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

UserDeviceSubroutine::UserDeviceSubroutine (std::string const & subroutineName,
    SgScopeStatement * moduleScope, Declarations & declarations) :
  Subroutine (subroutineName + "_device")
{
  userHostSubroutineName = subroutineName;

  copyAndModifySubroutine (moduleScope, declarations);
}
