#include <CPPModifyOP2CallsToComplyWithOxfordAPI.h>
#include <CPPOP2Definitions.h>
#include <Debug.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPModifyOP2CallsToComplyWithOxfordAPI::patchOpDeclareConstCall (
    SgExpressionPtrList & actualArguments)
{
  using SageBuilder::buildStringVal;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Patching call to OP_DECL_CONST", 5);

  SgAddressOfOp * addressOfOperator = isSgAddressOfOp (
      actualArguments[CPPImperialOpConstDefinition::index_OpDatName]);

  string variableName;

  if (addressOfOperator != NULL)
  {
    SgVarRefExp * operandExpression = isSgVarRefExp (
        addressOfOperator->get_operand ());

    ROSE_ASSERT (operandExpression != NULL);

    variableName = operandExpression->get_symbol ()->get_name ().getString ();
  }
  else
  {
    variableName
        = isSgVarRefExp (
            actualArguments[CPPImperialOpConstDefinition::index_OpDatName])->get_symbol ()->get_name ().getString ();
  }

  /*
   * ======================================================
   * Sandwich the name of the type in the 2nd position of
   * the argument list
   * ======================================================
   */
  vector <SgExpression *>::iterator position = actualArguments.begin () + 1;

  if (declarations->isTypeShort (variableName))
  {
    actualArguments.insert (position, buildStringVal ("short"));
  }
  else if (declarations->isTypeInteger (variableName))
  {
    actualArguments.insert (position, buildStringVal ("int"));
  }
  else if (declarations->isTypeLong (variableName))
  {
    actualArguments.insert (position, buildStringVal ("long"));
  }
  else if (declarations->isTypeFloat (variableName))
  {
    actualArguments.insert (position, buildStringVal ("float"));
  }
  else if (declarations->isTypeDouble (variableName))
  {
    actualArguments.insert (position, buildStringVal ("double"));
  }
  else
  {
    Debug::getInstance ()->errorMessage (
        "Cannot ascertain base type of OP_CONST '" + variableName + "'");
  }
}

void
CPPModifyOP2CallsToComplyWithOxfordAPI::patchOpDeclareDatCall (
    SgVariableDeclaration * variableDeclaration, std::string const variableName)
{
  using SageBuilder::buildStringVal;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Patching call to OP_DECL_DAT", 5);

  SgAssignInitializer * assignmentInitializer = isSgAssignInitializer (
      variableDeclaration->get_decl_item (variableName)->get_initializer ());

  ROSE_ASSERT (assignmentInitializer != NULL);

  SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (
      assignmentInitializer->get_operand ());

  ROSE_ASSERT (functionCallExpression != NULL);

  SgExpressionPtrList & actualArguments =
      functionCallExpression->get_args ()->get_expressions ();

  /*
   * ======================================================
   * Sandwich the name of the OP_DAT base type in between the
   * dimension and the last argument
   * ======================================================
   */

  string const
      dataVariableName =
          isSgVarRefExp (
              actualArguments[CPPImperialOpDatDefinition::index_data])->get_symbol ()->get_name ().getString ();

  vector <SgExpression *>::iterator positionOfDataType =
      actualArguments.begin () + 2;

  if (declarations->isTypeShort (dataVariableName))
  {
    actualArguments.insert (positionOfDataType, buildStringVal ("short"));
  }
  else if (declarations->isTypeInteger (dataVariableName))
  {
    actualArguments.insert (positionOfDataType, buildStringVal ("int"));
  }
  else if (declarations->isTypeLong (dataVariableName))
  {
    actualArguments.insert (positionOfDataType, buildStringVal ("long"));
  }
  else if (declarations->isTypeFloat (dataVariableName))
  {
    actualArguments.insert (positionOfDataType, buildStringVal ("float"));
  }
  else if (declarations->isTypeDouble (dataVariableName))
  {
    actualArguments.insert (positionOfDataType, buildStringVal ("double"));
  }
  else
  {
    Debug::getInstance ()->errorMessage ("Cannot ascertain base type of data '"
        + dataVariableName + "'");
  }

  /*
   * ======================================================
   * Append the name of the OP_MAP to the end of the argument
   * list
   * ======================================================
   */

  vector <SgExpression *>::iterator positionOfVariableName =
      actualArguments.end ();

  actualArguments.insert (positionOfVariableName, buildStringVal (variableName));
}

void
CPPModifyOP2CallsToComplyWithOxfordAPI::patchOpDeclareMapCall (
    SgVariableDeclaration * variableDeclaration, std::string const variableName)
{
  using SageBuilder::buildStringVal;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Patching call to OP_DECL_MAP", 5);

  SgAssignInitializer * assignmentInitializer = isSgAssignInitializer (
      variableDeclaration->get_decl_item (variableName)->get_initializer ());

  ROSE_ASSERT (assignmentInitializer != NULL);

  SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (
      assignmentInitializer->get_operand ());

  ROSE_ASSERT (functionCallExpression != NULL);

  SgExpressionPtrList & actualArguments =
      functionCallExpression->get_args ()->get_expressions ();

  vector <SgExpression *>::iterator position = actualArguments.end ();

  /*
   * ======================================================
   * Append the name of the OP_MAP to the end of the argument
   * list
   * ======================================================
   */

  actualArguments.insert (position, buildStringVal (variableName));
}

void
CPPModifyOP2CallsToComplyWithOxfordAPI::patchOpDeclareSetCall (
    SgVariableDeclaration * variableDeclaration, std::string const variableName)
{
  using SageBuilder::buildStringVal;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Patching call to OP_DECL_SET", 5);

  SgAssignInitializer * assignmentInitializer = isSgAssignInitializer (
      variableDeclaration->get_decl_item (variableName)->get_initializer ());

  ROSE_ASSERT (assignmentInitializer != NULL);

  SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (
      assignmentInitializer->get_operand ());

  ROSE_ASSERT (functionCallExpression != NULL);

  SgExpressionPtrList & actualArguments =
      functionCallExpression->get_args ()->get_expressions ();

  vector <SgExpression *>::iterator position = actualArguments.end ();

  /*
   * ======================================================
   * Append the name of the OP_SET to the end of the argument
   * list
   * ======================================================
   */

  actualArguments.insert (position, buildStringVal (variableName));
}

void
CPPModifyOP2CallsToComplyWithOxfordAPI::visit (SgNode * node)
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
        string const typeName =
            isSgTypedefType (type)->get_name ().getString ();

        if (iequals (typeName, OP2::OP_SET))
        {
          patchOpDeclareSetCall (variableDeclaration, variableName);
        }
        else if (iequals (typeName, OP2::OP_MAP))
        {
          patchOpDeclareMapCall (variableDeclaration, variableName);
        }
        else if (iequals (typeName, OP2::OP_DAT))
        {
          patchOpDeclareDatCall (variableDeclaration, variableName);
        }
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

      if (iequals (calleeName, OP2::OP_DECL_CONST))
      {
        /*
         * ======================================================
         * An OP_CONST variable defined through an OP_DECL_CONST call
         * ======================================================
         */

        patchOpDeclareConstCall (
            functionCallExp->get_args ()->get_expressions ());
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

CPPModifyOP2CallsToComplyWithOxfordAPI::CPPModifyOP2CallsToComplyWithOxfordAPI (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  declarations (declarations)
{
  traverseInputFiles (project, preorder);
}
