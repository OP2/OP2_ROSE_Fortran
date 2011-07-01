#include <FortranDeclarations.h>
#include <CommonNamespaces.h>
#include <FortranOP2Variables.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranDeclarations::visit (SgNode * node)
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
          + procedureHeaderStatement->get_name ().getString () + "'", 8);

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

        FortranOpSetDefinition * opSetDeclaration =
            new FortranOpSetDefinition (actualArguments);

        OpSetDeclarations.push_back (opSetDeclaration);
      }
      else if (iequals (calleeName, OP2::OP_DECL_MAP))
      {
        /*
         * ======================================================
         * An OP_MAP variable declared through an OP_DECL_MAP call
         * ======================================================
         */

        FortranOpMapDefinition * opMapDeclaration =
            new FortranOpMapDefinition (actualArguments);

        OpMapDeclarations.push_back (opMapDeclaration);
      }
      else if (iequals (calleeName, OP2::OP_DECL_DAT))
      {
        /*
         * ======================================================
         * An OP_DAT variable declared through an OP_DECL_DAT call
         * ======================================================
         */

        FortranOpDatDefinition * opDatDeclaration =
            new FortranOpDatDefinition (actualArguments);

        if (isSgArrayType (opDatDeclaration->getActualType ()) == false)
        {
          Debug::getInstance ()->errorMessage ("OP_DAT variable '"
              + opDatDeclaration->getVariableName ()
              + "' is not an array type. Currently not supported.");
        }

        OpDatDeclarations.push_back (opDatDeclaration);
      }
      else if (iequals (calleeName, OP2::OP_DECL_GBL))
      {
        /*
         * ======================================================
         * An OP_DAT variable declared through an OP_DECL_GBL call
         * ======================================================
         */

        FortranOpGblDefinition * opGblDeclaration =
            new FortranOpGblDefinition (actualArguments);

        OpGblDeclarations.push_back (opGblDeclaration);
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

FortranDeclarations::FortranDeclarations (SgProject * project)
{
  traverseInputFiles (project, preorder);
}
