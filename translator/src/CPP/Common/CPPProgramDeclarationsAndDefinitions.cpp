#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPOP2Definitions.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPProgramDeclarationsAndDefinitions::detectAndHandleOP2Definition (
    SgVariableDeclaration * variableDeclaration,
    std::string const variableName, SgTypedefType * typeDefinition)
{
  using boost::iequals;
  using std::string;

  string const typeName = typeDefinition->get_name ().getString ();

  if (iequals (typeName, OP2::OP_SET))
  {
    SgAssignInitializer * assignmentInitializer = isSgAssignInitializer (
        variableDeclaration->get_decl_item (variableName)->get_initializer ());

    ROSE_ASSERT (assignmentInitializer != NULL);

    SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (
        assignmentInitializer->get_operand ());

    ROSE_ASSERT (functionCallExpression != NULL);

    SgExpressionPtrList & actualArguments =
        functionCallExpression->get_args ()->get_expressions ();

    CPPImperialOpSetDefinition * opSetDeclaration =
        new CPPImperialOpSetDefinition (actualArguments, variableName);

    OpSetDefinitions.push_back (opSetDeclaration);
  }
  else if (iequals (typeName, OP2::OP_MAP))
  {
    SgAssignInitializer * assignmentInitializer = isSgAssignInitializer (
        variableDeclaration->get_decl_item (variableName)->get_initializer ());

    ROSE_ASSERT (assignmentInitializer != NULL);

    SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (
        assignmentInitializer->get_operand ());

    ROSE_ASSERT (functionCallExpression != NULL);

    SgExpressionPtrList & actualArguments =
        functionCallExpression->get_args ()->get_expressions ();

    CPPImperialOpMapDefinition * opMapDeclaration =
        new CPPImperialOpMapDefinition (actualArguments, variableName);

    OpMapDefinitions.push_back (opMapDeclaration);
  }
  else if (iequals (typeName, OP2::OP_DAT))
  {
    SgAssignInitializer * assignmentInitializer = isSgAssignInitializer (
        variableDeclaration->get_decl_item (variableName)->get_initializer ());

    ROSE_ASSERT (assignmentInitializer != NULL);

    SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (
        assignmentInitializer->get_operand ());

    ROSE_ASSERT (functionCallExpression != NULL);

    SgExpressionPtrList & actualArguments =
        functionCallExpression->get_args ()->get_expressions ();

    CPPImperialOpDatDefinition * opDatDeclaration =
        new CPPImperialOpDatDefinition (actualArguments, variableName);

    OpDatDefinitions.push_back (opDatDeclaration);
  }
}

void
CPPProgramDeclarationsAndDefinitions::visit (SgNode * node)
{
  using boost::iequals;
  using std::string;

  switch (node->variantT ())
  {
    case V_SgVariableDeclaration:
    {
      SgVariableDeclaration * variableDeclaration = isSgVariableDeclaration (
          node);

      string const
          variableName =
              variableDeclaration->get_variables ().front ()->get_name ().getString ();

      SgType * type =
          variableDeclaration->get_decl_item (variableName)->get_type ();

      if (isSgTypedefType (type) != NULL)
      {
        detectAndHandleOP2Definition (variableDeclaration, variableName,
            isSgTypedefType (type));
      }
      else if (isSgTypeInt (type) != NULL)
      {

      }

      break;
    }

    case V_SgFunctionCallExp:
    {
      /*
       * ======================================================
       * Function call found in the AST. Get its actual arguments
       * and the callee name
       * ======================================================
       */
      SgFunctionCallExp * functionCallExp = isSgFunctionCallExp (node);

      SgExpressionPtrList & actualArguments =
          functionCallExp->get_args ()->get_expressions ();

      string const
          calleeName =
              functionCallExp->getAssociatedFunctionSymbol ()->get_name ().getString ();

      Debug::getInstance ()->debugMessage ("Found function call to '"
          + calleeName + "'", 5);

      if (iequals (calleeName, OP2::OP_DECL_CONST))
      {
        /*
         * ======================================================
         * An OP_CONST variable defined through an OP_DECL_CONST call
         * ======================================================
         */

        CPPImperialOpConstDefinition * opConstDeclaration =
            new CPPImperialOpConstDefinition (actualArguments);

        OpConstDefinitions.push_back (opConstDeclaration);
      }

      break;
    }

    default:
    {
      break;
    }
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPProgramDeclarationsAndDefinitions::CPPProgramDeclarationsAndDefinitions (
    SgProject * project)
{
  traverseInputFiles (project, preorder);
}
