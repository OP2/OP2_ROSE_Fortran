#include <boost/algorithm/string.hpp>

#include "OP2DeclaredVariables.h"
#include "OP2CommonDefinitions.h"
#include "Debug.h"

void
OP2DeclaredVariables::visit (SgNode * node)
{
  using boost::iequals;
  using std::string;
  using std::pair;
  using std::make_pair;

  if (node->variantT () == V_SgFunctionCallExp)
  {
    /*
     * Function call found in the AST. Get its arguments and its name
     */
    SgFunctionCallExp * functionCallExp = isSgFunctionCallExp (node);

    SgExpressionPtrList & actualArguments =
        functionCallExp->get_args ()->get_expressions ();

    string const
        calleeName =
            functionCallExp->getAssociatedFunctionSymbol ()->get_name ().getString ();

    if (iequals (calleeName, OP2::OP_DECL_SET_PREFIX))
    {
      /*
       * Associate the OP_SET variable with its arguments in an OP_DECL_SET call
       */

      SgVarRefExp * opSetReference = isSgVarRefExp (
          actualArguments[OP2::INDEX_OF_OP_SET_IN_OP_DECL_SET_PARAMETER_LIST]);
      ROSE_ASSERT (opSetReference != NULL);

      string const opSetName =
          opSetReference->get_symbol ()->get_name ().getString ();

      OP_SET_Declarations.insert (make_pair (opSetName, actualArguments));

      Debug::getInstance ()->debugMessage ("'" + OP2::OP_DECL_SET_PREFIX
          + "' call found with OP_SET variable '" + opSetName + "'", 5);
    }
    else if (iequals (calleeName, OP2::OP_DECL_MAP_PREFIX))
    {
      /*
       * Associate the OP_MAP variable with its arguments in an OP_DECL_MAP call
       */

      SgVarRefExp * opMapReference = isSgVarRefExp (
          actualArguments[OP2::INDEX_OF_OP_MAP_IN_OP_DECL_MAP_PARAMETER_LIST]);
      ROSE_ASSERT (opMapReference != NULL);

      string const opMapName =
          opMapReference->get_symbol ()->get_name ().getString ();

      OP_MAP_Declarations.insert (make_pair (opMapName, actualArguments));

      Debug::getInstance ()->debugMessage ("'" + OP2::OP_DECL_MAP_PREFIX
          + "' call found with OP_MAP variable '" + opMapName + "'", 5);
    }
    else if (iequals (calleeName, OP2::OP_DECL_DAT_PREFIX))
    {
      /*
       * Associate the OP_DAT variable with its arguments in an OP_DECL_DAT call
       */

      SgVarRefExp * opDatReference = isSgVarRefExp (
          actualArguments[OP2::INDEX_OF_OP_DAT_IN_OP_DECL_DAT_PARAMETER_LIST]);
      ROSE_ASSERT (opDatReference != NULL);

      string const opDatName =
          opDatReference->get_symbol ()->get_name ().getString ();

      OP_DAT_Declarations.insert (make_pair (opDatName, actualArguments));

      Debug::getInstance ()->debugMessage ("'" + OP2::OP_DECL_DAT_PREFIX
          + "' call found with OP_DAT variable '" + opDatName + "'", 5);
    }
    else if (iequals (calleeName, OP2::OP_DECL_GBL_PREFIX))
    {
      /*
       * Associate the OP_DAT variable with its arguments in an OP_DECL_GBL call
       */

      SgVarRefExp * opDatReference = isSgVarRefExp (
          actualArguments[OP2::INDEX_OF_OP_DAT_IN_OP_DECL_GBL_PARAMETER_LIST]);
      ROSE_ASSERT (opDatReference != NULL);

      string const opDatName =
          opDatReference->get_symbol ()->get_name ().getString ();

      OP_GBL_Declarations.insert (make_pair (opDatName, actualArguments));

      Debug::getInstance ()->debugMessage ("'" + OP2::OP_DECL_GBL_PREFIX
          + "' call found with OP_DAT variable '" + opDatName + "'", 5);
    }
  }

}
