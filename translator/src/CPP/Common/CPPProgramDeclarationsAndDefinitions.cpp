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
CPPProgramDeclarationsAndDefinitions::handleBaseTypeDeclaration (SgType * type,
    std::string const & variableName)
{
  if (isSgTypeShort (type) != NULL)
  {
    Debug::getInstance ()->debugMessage ("'" + variableName + "' is a short",
        Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    shortDeclarations.push_back (variableName);
  }
  else if (isSgTypeInt (type) != NULL)
  {
    Debug::getInstance ()->debugMessage (
        "'" + variableName + "' is an integer", Debug::FUNCTION_LEVEL,
        __FILE__, __LINE__);

    integerDeclarations.push_back (variableName);
  }
  else if (isSgTypeLong (type) != NULL)
  {
    Debug::getInstance ()->debugMessage ("'" + variableName + "' is a long",
        Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    longDeclarations.push_back (variableName);
  }
  else if (isSgTypeFloat (type) != NULL)
  {
    Debug::getInstance ()->debugMessage ("'" + variableName + "' is a float",
        Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    floatDeclarations.push_back (variableName);
  }
  else if (isSgTypeDouble (type) != NULL)
  {
    Debug::getInstance ()->debugMessage ("'" + variableName + "' is a double",
        Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    doubleDeclarations.push_back (variableName);
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
      else if (isSgPointerType (type) != NULL)
      {
        Debug::getInstance ()->debugMessage ("'" + variableName
            + "' is a pointer", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

        handleBaseTypeDeclaration (isSgPointerType (type)->get_base_type (),
            variableName);
      }
      else if (isSgArrayType (type) != NULL)
      {
        Debug::getInstance ()->debugMessage ("'" + variableName
            + "' is a array", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

        handleBaseTypeDeclaration (isSgArrayType (type)->get_base_type (),
            variableName);
      }
      else
      {
        handleBaseTypeDeclaration (type, variableName);
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

      string const
          calleeName =
              functionCallExp->getAssociatedFunctionSymbol ()->get_name ().getString ();

      Debug::getInstance ()->debugMessage ("Found function call to '"
          + calleeName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

      if (iequals (calleeName, OP2::OP_DECL_CONST))
      {
        /*
         * ======================================================
         * An OP_CONST variable defined through an OP_DECL_CONST call
         * ======================================================
         */

        CPPImperialOpConstDefinition * opConstDeclaration =
            new CPPImperialOpConstDefinition (
                functionCallExp->get_args ()->get_expressions ());

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
