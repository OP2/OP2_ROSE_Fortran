#include <CPPModifyOP2CallsToComplyWithOxfordAPI.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPOP2Definitions.h>
#include <Debug.h>
#include <Exceptions.h>
#include <OP2Definitions.h>

void
CPPModifyOP2CallsToComplyWithOxfordAPI::patchOpArgGblCall (
    SgExpressionPtrList & actualArguments)
{
  using namespace SageBuilder;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Patching call to OP_ARG_GBL",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgAddressOfOp * addressOfOperator = isSgAddressOfOp (
      actualArguments[CPPImperialOpArgGblCall::indexOpDat]);

  string opConstVariableName;

  if (addressOfOperator != NULL)
  {
    SgVarRefExp * operandExpression = isSgVarRefExp (
        addressOfOperator->get_operand ());

    ROSE_ASSERT (operandExpression != NULL);

    opConstVariableName
        = operandExpression->get_symbol ()->get_name ().getString ();
  }
  else
  {
    opConstVariableName
        = isSgVarRefExp (actualArguments[CPPImperialOpArgGblCall::indexOpDat])->get_symbol ()->get_name ().getString ();
  }

  if (declarations->isTypeShort (opConstVariableName))
  {
    /*
     * ======================================================
     * Put the dimension of the OP_CONST after the OP_DAT
     * reference
     * ======================================================
     */

    vector <SgExpression *>::iterator positionOfDimension =
        actualArguments.begin () + 1;

    actualArguments.insert (positionOfDimension, buildIntVal (1));

    /*
     * ======================================================
     * Put the type of the OP_DAT after the dimension of the
     * OP_DAT
     * ======================================================
     */

    vector <SgExpression *>::iterator positionOfTypeName =
        actualArguments.begin () + 2;

    actualArguments.insert (positionOfTypeName, buildStringVal ("short"));
  }
  else if (declarations->isTypeInteger (opConstVariableName))
  {
    /*
     * ======================================================
     * Put the dimension of the OP_CONST after the OP_DAT
     * reference
     * ======================================================
     */

    vector <SgExpression *>::iterator positionOfDimension =
        actualArguments.begin () + 1;

    actualArguments.insert (positionOfDimension, buildIntVal (1));

    /*
     * ======================================================
     * Put the type of the OP_DAT after the dimension of the
     * OP_DAT
     * ======================================================
     */

    vector <SgExpression *>::iterator positionOfTypeName =
        actualArguments.begin () + 2;

    actualArguments.insert (positionOfTypeName, buildStringVal ("int"));
  }
  else if (declarations->isTypeLong (opConstVariableName))
  {
    /*
     * ======================================================
     * Put the dimension of the OP_CONST after the OP_DAT
     * reference
     * ======================================================
     */

    vector <SgExpression *>::iterator positionOfDimension =
        actualArguments.begin () + 1;

    actualArguments.insert (positionOfDimension, buildIntVal (1));

    /*
     * ======================================================
     * Put the type of the OP_DAT after the dimension of the
     * OP_DAT
     * ======================================================
     */

    vector <SgExpression *>::iterator positionOfTypeName =
        actualArguments.begin () + 2;

    actualArguments.insert (positionOfTypeName, buildStringVal ("long"));
  }
  else if (declarations->isTypeFloat (opConstVariableName))
  {
    /*
     * ======================================================
     * Put the dimension of the OP_CONST after the OP_DAT
     * reference
     * ======================================================
     */

    vector <SgExpression *>::iterator positionOfDimension =
        actualArguments.begin () + 1;

    actualArguments.insert (positionOfDimension, buildIntVal (1));

    /*
     * ======================================================
     * Put the type of the OP_DAT after the dimension of the
     * OP_DAT
     * ======================================================
     */

    vector <SgExpression *>::iterator positionOfTypeName =
        actualArguments.begin () + 2;

    actualArguments.insert (positionOfTypeName, buildStringVal ("float"));
  }
  else if (declarations->isTypeDouble (opConstVariableName))
  {
    /*
     * ======================================================
     * Put the dimension of the OP_CONST after the OP_DAT
     * reference
     * ======================================================
     */

    vector <SgExpression *>::iterator positionOfDimension =
        actualArguments.begin () + 1;

    actualArguments.insert (positionOfDimension, buildIntVal (1));

    /*
     * ======================================================
     * Put the type of the OP_DAT after the dimension of the
     * OP_DAT
     * ======================================================
     */

    vector <SgExpression *>::iterator positionOfTypeName =
        actualArguments.begin () + 2;

    actualArguments.insert (positionOfTypeName, buildStringVal ("double"));
  }
  else
  {
    throw Exceptions::ParallelLoop::UnsupportedBaseTypeException (
        "Cannot ascertain type of OP_CONST '" + opConstVariableName + "'");
  }
}

void
CPPModifyOP2CallsToComplyWithOxfordAPI::patchOpArgDatCall (
    SgExpressionPtrList & actualArguments)
{
  using namespace SageBuilder;
  using boost::iequals;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Patching call to OP_ARG_DAT",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVarRefExp * opDatReference = isSgVarRefExp (
      actualArguments[CPPImperialOpArgDatCall::indexOpDat]);

  ROSE_ASSERT (opDatReference != NULL);

  string const opDatVariableName =
      opDatReference ->get_symbol ()->get_name ().getString ();

  CPPImperialOpDatDefinition
      * opDatDefinition =
          static_cast <CPPImperialOpDatDefinition *> (declarations->getOpDatDefinition (
              opDatVariableName));

  /*
   * ======================================================
   * Put the dimension of the OP_DAT after the OP_MAP
   * reference
   * ======================================================
   */

  vector <SgExpression *>::iterator positionOfDimension =
      actualArguments.begin () + 3;

  unsigned int dimension = opDatDefinition->getDimension ();

  actualArguments.insert (positionOfDimension, buildIntVal (dimension));

  /*
   * ======================================================
   * Put the type of the OP_DAT after the dimension of the
   * OP_DAT
   * ======================================================
   */

  vector <SgExpression *>::iterator positionOfTypeName =
      actualArguments.begin () + 4;

  SgPointerType * pointerType = isSgPointerType (
      opDatDefinition->getPrimitiveType ());

  ROSE_ASSERT (pointerType != NULL);

  if (isSgTypeShort (pointerType->get_base_type ()) != NULL)
  {
    actualArguments.insert (positionOfTypeName, buildStringVal ("short"));
  }
  else if (isSgTypeInt (pointerType->get_base_type ()) != NULL)
  {
    actualArguments.insert (positionOfTypeName, buildStringVal ("int"));
  }
  else if (isSgTypeLong (pointerType->get_base_type ()) != NULL)
  {
    actualArguments.insert (positionOfTypeName, buildStringVal ("long"));
  }
  else if (isSgTypeFloat (pointerType->get_base_type ()) != NULL)
  {
    actualArguments.insert (positionOfTypeName, buildStringVal ("float"));
  }
  else if (isSgTypeDouble (pointerType->get_base_type ()) != NULL)
  {
    actualArguments.insert (positionOfTypeName, buildStringVal ("double"));
  }
  else
  {
    throw Exceptions::ParallelLoop::UnsupportedBaseTypeException (
        "Cannot ascertain base type of OP_DAT '" + opDatVariableName + "'");
  }
}

void
CPPModifyOP2CallsToComplyWithOxfordAPI::patchOpParLoopCall (
    SgExpressionPtrList & actualArguments)
{
  using namespace SageBuilder;
  using boost::iequals;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Patching call to OP_PAR_LOOP",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgFunctionRefExp * userSubroutineExpression = isSgFunctionRefExp (
      actualArguments.front ());

  /*
   * ======================================================
   * Put the name of the user subroutine  after the reference
   * to the subroutine
   * ======================================================
   */

  vector <SgExpression *>::iterator positionOfFunctionName =
      actualArguments.begin () + 1;

  std::string const
      userSubroutineName =
          userSubroutineExpression->getAssociatedFunctionDeclaration ()->get_name ().getString ();

  actualArguments.insert (positionOfFunctionName, buildStringVal (
      userSubroutineName));

  /*
   * ======================================================
   * Now modify OP_ARG_DAT arguments
   * ======================================================
   */

  for (vector <SgExpression *>::iterator it = actualArguments.begin (); it
      != actualArguments.end (); ++it)
  {
    SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (*it);

    if (functionCallExpression != NULL)
    {
      std::string const
          functionCallName =
              functionCallExpression->getAssociatedFunctionDeclaration ()->get_name ().getString ();

      if (iequals (functionCallName, OP2::OP_ARG_DAT))
      {
        patchOpArgDatCall (
            functionCallExpression->get_args ()->get_expressions ());
      }
      else if (iequals (functionCallName, OP2::OP_ARG_GBL))
      {
        patchOpArgGblCall (
            functionCallExpression->get_args ()->get_expressions ());
      }
      else
      {
        throw Exceptions::CodeGeneration::UnknownSubroutineException (
            "Unknown function call '" + functionCallName + "' in OP_PAR_LOOP");
      }
    }
  }
}

void
CPPModifyOP2CallsToComplyWithOxfordAPI::patchOpDeclareConstCall (
    SgExpressionPtrList & actualArguments)
{
  using namespace SageBuilder;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Patching call to OP_DECL_CONST",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgAddressOfOp * addressOfOperator = isSgAddressOfOp (
      actualArguments[CPPImperialOpConstDefinition::indexData]);

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
            actualArguments[CPPImperialOpConstDefinition::indexData])->get_symbol ()->get_name ().getString ();
  }

  /*
   * ======================================================
   * Sandwich the name of the type in between the constant
   * value and the last argument
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
    throw Exceptions::ParallelLoop::UnsupportedBaseTypeException (
        "Cannot ascertain base type of OP_CONST '" + variableName + "'");
  }
}

void
CPPModifyOP2CallsToComplyWithOxfordAPI::patchOpDeclareDatCall (
    SgVariableDeclaration * variableDeclaration, std::string const variableName)
{
  using namespace SageBuilder;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Patching call to OP_DECL_DAT",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

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
              actualArguments[CPPImperialOpDatDefinition::indexDataArray])->get_symbol ()->get_name ().getString ();

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
    throw Exceptions::ParallelLoop::UnsupportedBaseTypeException (
        "Cannot ascertain base type of data '" + dataVariableName + "'");
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
  using namespace SageBuilder;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Patching call to OP_DECL_MAP",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

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
  using namespace SageBuilder;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Patching call to OP_DECL_SET",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

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
      else if (iequals (calleeName, OP2::OP_PAR_LOOP))
      {
        patchOpParLoopCall (functionCallExp->get_args ()->get_expressions ());
      }

      break;
    }

    default:
    {
      break;
    }
  }
}

CPPModifyOP2CallsToComplyWithOxfordAPI::CPPModifyOP2CallsToComplyWithOxfordAPI (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  declarations (declarations)
{
  traverseInputFiles (project, preorder);
}
