#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPOP2Definitions.h>
#include <CPPParallelLoop.h>
#include <CommonNamespaces.h>
#include <boost/filesystem.hpp>

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

    CPPImperialOpSetDefinition * opSetDeclaration =
        new CPPImperialOpSetDefinition (functionCallExpression->get_args (),
            variableName);

    OpSetDefinitions[opSetDeclaration->getVariableName ()] = opSetDeclaration;
  }
  else if (iequals (typeName, OP2::OP_MAP))
  {
    SgAssignInitializer * assignmentInitializer = isSgAssignInitializer (
        variableDeclaration->get_decl_item (variableName)->get_initializer ());

    ROSE_ASSERT (assignmentInitializer != NULL);

    SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (
        assignmentInitializer->get_operand ());

    ROSE_ASSERT (functionCallExpression != NULL);

    CPPImperialOpMapDefinition * opMapDeclaration =
        new CPPImperialOpMapDefinition (functionCallExpression->get_args (),
            variableName);

    OpMapDefinitions[opMapDeclaration->getVariableName ()] = opMapDeclaration;
  }
  else if (iequals (typeName, OP2::OP_DAT))
  {
    SgAssignInitializer * assignmentInitializer = isSgAssignInitializer (
        variableDeclaration->get_decl_item (variableName)->get_initializer ());

    ROSE_ASSERT (assignmentInitializer != NULL);

    SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (
        assignmentInitializer->get_operand ());

    ROSE_ASSERT (functionCallExpression != NULL);

    CPPImperialOpDatDefinition * opDatDeclaration =
        new CPPImperialOpDatDefinition (functionCallExpression->get_args (),
            variableName);

    OpDatDefinitions[opDatDeclaration->getVariableName ()] = opDatDeclaration;
  }
}

void
CPPProgramDeclarationsAndDefinitions::visit (SgNode * node)
{
  using boost::filesystem::path;
  using boost::filesystem::system_complete;
  using boost::iequals;
  using std::string;

  switch (node->variantT ())
  {
    case V_SgSourceFile:
    {
      SgSourceFile * sourceFile = isSgSourceFile (node);

      path p = system_complete (path (sourceFile->getFileName ()));

      currentSourceFile = p.filename ();

      Debug::getInstance ()->debugMessage ("Source file '" + currentSourceFile
          + "' detected", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__ );

      break;
    }
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

    case V_SgFunctionDefinition:
    {
      SgFunctionDefinition * functionDeclaration =
          isSgFunctionDefinition (node);

      Debug::getInstance ()->debugMessage ("Found definition: "
          + functionDeclaration->get_declaration ()->get_name ().getString (),
          Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

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
            new CPPImperialOpConstDefinition (functionCallExp->get_args ());

        OpConstDefinitions[opConstDeclaration->getVariableName ()]
            = opConstDeclaration;
      }
      else if (iequals (calleeName, OP2::OP_PAR_LOOP))
      {
        SgExpressionPtrList & actualArguments =
            functionCallExp->get_args ()->get_expressions ();

        SgFunctionRefExp * functionRefExpression = isSgFunctionRefExp (
            actualArguments.front ());

        ROSE_ASSERT (functionRefExpression != NULL);

        string const
            userSubroutineName =
                functionRefExpression->getAssociatedFunctionDeclaration ()->get_name ().getString ();

        Debug::getInstance ()->debugMessage ("Found '" + calleeName
            + "' with (host) user subroutine '" + userSubroutineName + "'",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        if (parallelLoops.find (userSubroutineName) == parallelLoops.end ())
        {
          /*
           * ======================================================
           * If this kernel has not been previously encountered then
           * build a new parallel loop representation
           * ======================================================
           */

          CPPParallelLoop * parallelLoop = new CPPParallelLoop (
              functionCallExp, functionCallExp->getFilenameString ());

          parallelLoops[userSubroutineName] = parallelLoop;
        }
        else
        {
          Debug::getInstance ()->debugMessage ("Parallel loop for '"
              + userSubroutineName + "' already created",
              Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
        }

      }

      break;
    }

    default:
    {
      break;
    }
  }
}

CPPProgramDeclarationsAndDefinitions::CPPProgramDeclarationsAndDefinitions (
    SgProject * project)
{
  traverse (project, preorder);
}
