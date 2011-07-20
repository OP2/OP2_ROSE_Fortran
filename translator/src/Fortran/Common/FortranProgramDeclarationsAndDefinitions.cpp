#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranOP2Definitions.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranProgramDeclarationsAndDefinitions::visit (SgNode * node)
{
  using boost::iequals;
  using boost::starts_with;
  using std::string;

  switch (node->variantT ())
  {
    case V_SgProcedureHeaderStatement:
    {
      /*
       * ======================================================
       * We need to store all subroutine definitions since we
       * later have to copy and modify the user kernel subroutine
       * ======================================================
       */
      SgProcedureHeaderStatement * procedureHeaderStatement =
          isSgProcedureHeaderStatement (node);

      subroutinesInSourceCode.push_back (procedureHeaderStatement);

      Debug::getInstance ()->debugMessage ("Found procedure header statement '"
          + procedureHeaderStatement->get_name ().getString () + "'",
          Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

      break;
    }

    case V_SgVariableDeclaration:
    {
      SgVariableDeclaration * variableDeclaration = isSgVariableDeclaration (
          node);

      for (SgInitializedNamePtrList::iterator it =
          variableDeclaration->get_variables ().begin (); it
          != variableDeclaration->get_variables ().end (); it++)
      {
        string const variableName = (*it)->get_name ().getString ();

        SgType * type =
            variableDeclaration->get_decl_item (variableName)->get_type ();

        handleBaseTypeDeclaration (type, variableName);

        SgAssignInitializer * assignmentInitializer =
            isSgAssignInitializer (variableDeclaration->get_decl_item (
                variableName)->get_initializer ());

        if (assignmentInitializer != NULL)
        {
          addInitializer (variableName, assignmentInitializer->get_operand ());
        }
      }

      break;
    }

    case V_SgAssignOp:
    {
      SgAssignOp * assignExpression = isSgAssignOp (node);

      SgExpression * lhs = assignExpression->get_lhs_operand ();

      SgExpression * rhs = assignExpression->get_rhs_operand ();

      if (isSgVarRefExp (lhs) != NULL)
      {
        SgVarRefExp * lhsReference = isSgVarRefExp (lhs);

        string const variableName =
            lhsReference->get_symbol ()->get_name ().getString ();

        Debug::getInstance ()->debugMessage ("Storing '"
            + rhs->unparseToString () + "' as assignment expression for '"
            + variableName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        addInitializer (variableName, rhs);
      }
      else if (isSgPntrArrRefExp (lhs))
      {
        SgPntrArrRefExp * lhsReference = isSgPntrArrRefExp (lhs);

        std::cout << lhsReference->get_lhs_operand ()->unparseToString ()
            << std::endl;
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

      if (iequals (calleeName, OP2::OP_DECL_SET))
      {
        /*
         * ======================================================
         * An OP_SET variable declared through an OP_DECL_SET call
         * ======================================================
         */

        FortranOpSetDefinition * opSetDeclaration = new FortranOpSetDefinition (
            actualArguments);

        OpSetDefinitions[opSetDeclaration->getVariableName ()]
            = opSetDeclaration;
      }
      else if (iequals (calleeName, OP2::OP_DECL_MAP))
      {
        /*
         * ======================================================
         * An OP_MAP variable declared through an OP_DECL_MAP call
         * ======================================================
         */

        FortranOpMapDefinition * opMapDeclaration = new FortranOpMapDefinition (
            actualArguments);

        OpMapDefinitions[opMapDeclaration->getVariableName ()]
            = opMapDeclaration;
      }
      else if (iequals (calleeName, OP2::OP_DECL_DAT))
      {
        /*
         * ======================================================
         * An OP_DAT variable declared through an OP_DECL_DAT call
         * ======================================================
         */

        FortranOpDatDefinition * opDatDeclaration = new FortranOpDatDefinition (
            actualArguments);

        if (isSgArrayType (opDatDeclaration->getPrimitiveType ()) == NULL)
        {
          Debug::getInstance ()->errorMessage ("OP_DAT variable '"
              + opDatDeclaration->getVariableName ()
              + "' is not an array type. Currently not supported.");
        }

        OpDatDefinitions[opDatDeclaration->getVariableName ()]
            = opDatDeclaration;
      }
      else if (iequals (calleeName, OP2::OP_DECL_GBL))
      {
        /*
         * ======================================================
         * An OP_DAT variable declared through an OP_DECL_GBL call
         * ======================================================
         */

        FortranOpGblDefinition * opGblDeclaration = new FortranOpGblDefinition (
            actualArguments);

        OpGblDefinitions[opGblDeclaration->getVariableName ()]
            = opGblDeclaration;
      }
      else if (iequals (calleeName, OP2::OP_DECL_CONST))
      {
        /*
         * ======================================================
         * A constant declared through an OP_DECL_CONST call
         * ======================================================
         */

        FortranOpConstDefinition * opConstDeclaration =
            new FortranOpConstDefinition (actualArguments);

        OpConstDefinitions[opConstDeclaration->getVariableName ()]
            = opConstDeclaration;
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

FortranProgramDeclarationsAndDefinitions::FortranProgramDeclarationsAndDefinitions (
    SgProject * project)
{
  traverseInputFiles (project, preorder);
}
