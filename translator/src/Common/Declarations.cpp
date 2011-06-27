#include <boost/algorithm/string.hpp>
#include <Declarations.h>
#include <CommonNamespaces.h>
#include <Debug.h>

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

OP_SET_Declaration *
Declarations::get_OP_SET_Declaration (std::string const & opSETName)
    throw (std::string const &)
{
  using boost::iequals;
  using std::vector;

  for (vector <OP_SET_Declaration *>::iterator it =
      OP_SET_Declarations.begin (); it != OP_SET_Declarations.end (); ++it)
  {
    if (iequals ((*it)->getVariableName (), opSETName))
    {
      return *it;
    }
  }

  throw opSETName;
}

OP_MAP_Declaration *
Declarations::get_OP_MAP_Declaration (std::string const & opMAPName)
    throw (std::string const &)
{
  using boost::iequals;
  using std::vector;

  for (vector <OP_MAP_Declaration *>::iterator it =
      OP_MAP_Declarations.begin (); it != OP_MAP_Declarations.end (); ++it)
  {
    if (iequals ((*it)->getVariableName (), opMAPName))
    {
      return *it;
    }
  }

  throw opMAPName;
}

OP_DAT_Declaration *
Declarations::get_OP_DAT_Declaration (std::string const & opDATName)
    throw (std::string const &)
{
  using boost::iequals;
  using std::vector;

  for (vector <OP_DAT_Declaration *>::iterator it =
      OP_DAT_Declarations.begin (); it != OP_DAT_Declarations.end (); ++it)
  {
    if (iequals ((*it)->getVariableName (), opDATName))
    {
      return *it;
    }
  }

  throw opDATName;
}

OP_GBL_Declaration *
Declarations::get_OP_GBL_Declaration (std::string const & opGBLName)
    throw (std::string const &)
{
  using boost::iequals;
  using std::vector;

  for (vector <OP_GBL_Declaration *>::iterator it =
      OP_GBL_Declarations.begin (); it != OP_GBL_Declarations.end (); ++it)
  {
    if (iequals ((*it)->getVariableName (), opGBLName))
    {
      return *it;
    }
  }

  throw opGBLName;
}

void
Declarations::visit (SgNode * node)
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

        OP_SET_Declaration * opSetDeclaration = new OP_SET_Declaration (
            actualArguments);

        OP_SET_Declarations.push_back (opSetDeclaration);
      }
      else if (iequals (calleeName, OP2::OP_DECL_MAP))
      {
        /*
         * ======================================================
         * An OP_MAP variable declared through an OP_DECL_MAP call
         * ======================================================
         */

        OP_MAP_Declaration * opMapDeclaration = new OP_MAP_Declaration (
            actualArguments);

        OP_MAP_Declarations.push_back (opMapDeclaration);
      }
      else if (iequals (calleeName, OP2::OP_DECL_DAT))
      {
        /*
         * ======================================================
         * An OP_DAT variable declared through an OP_DECL_DAT call
         * ======================================================
         */

        OP_DAT_Declaration * opDatDeclaration = new OP_DAT_Declaration (
            actualArguments);

        if (isSgArrayType (opDatDeclaration->getActualType ()) == false)
        {
          Debug::getInstance ()->errorMessage ("OP_DAT variable '"
              + opDatDeclaration->getVariableName ()
              + "' is not an array type. Currently not supported.");
        }

        OP_DAT_Declarations.push_back (opDatDeclaration);
      }
      else if (iequals (calleeName, OP2::OP_DECL_GBL))
      {
        /*
         * ======================================================
         * An OP_DAT variable declared through an OP_DECL_GBL call
         * ======================================================
         */

        OP_GBL_Declaration * opGblDeclaration = new OP_GBL_Declaration (
            actualArguments);

        OP_GBL_Declarations.push_back (opGblDeclaration);
      }

      break;
    }

    case V_SgVariableDeclaration:
    {
      SgVariableDeclaration * variableDeclaration = isSgVariableDeclaration (
          node);

      switch (variableDeclaration->get_variables ().front ()->get_type ()->variantT ())
      {
        case V_SgClassType:
        {
          SgClassType * classType = isSgClassType (
              variableDeclaration->get_variables ().front ()->get_type ());

          if (starts_with (classType->get_name ().getString (), OP2::OP_DAT))
          {

          }
          else if (starts_with (classType->get_name ().getString (),
              OP2::OP_DAT_GBL))
          {

          }

          break;
        }

        case V_SgTypedefType:
        {
          SgTypedefType * typedefType = isSgTypedefType (
              variableDeclaration->get_variables ().front ()->get_type ());

          if (iequals (typedefType->get_name ().getString (), OP2::OP_SET))
          {

          }

          break;
        }

        default:
        {
          break;
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

std::vector <SgProcedureHeaderStatement *>::const_iterator
Declarations::first_SubroutineInSourceCode ()
{
  return subroutinesInSourceCode.begin ();
}

std::vector <SgProcedureHeaderStatement *>::const_iterator
Declarations::last_SubroutineInSourceCode ()
{
  return subroutinesInSourceCode.end ();
}

Declarations::Declarations (SgProject * project) :
  project (project)
{
}
