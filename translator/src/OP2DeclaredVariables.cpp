#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include "OpDeclaredVariables.h"
#include "OP2CommonDefinitions.h"
#include "Debug.h"

void
OpDeclaredVariables::visit (SgNode * node)
{
  using std::string;
  using std::pair;

  if (node->variantT () == V_SgFunctionCallExp)
  {
    /*
     * Function call found in the AST.
     * Get its arguments and its name
     */
    SgFunctionCallExp * functionCallExp = isSgFunctionCallExp (node);

    SgExpressionPtrList & args =
        functionCallExp->get_args ()->get_expressions ();

    string const
        calleeName =
            functionCallExp->getAssociatedFunctionSymbol ()->get_name ().getString ();

    if (calleeName.compare (OP2::OP_DECL_SET_PREFIX) == 0)
    {
      /*
       * Add a new map associating the op_set variable reference with the arguments
       */
      SgVarRefExp * setRef = isSgVarRefExp (args[OP2::OP_DECL_SET_VAR_REF]);
      ROSE_ASSERT (setRef != NULL);

      OP_SET_Declarations.insert (pair <SgName, SgExpressionPtrList> (
          setRef->get_symbol ()->get_name (), args));

      Debug::getInstance ()->debugMessage (OP2::OP_DECL_SET_PREFIX
          + " call found: " + calleeName + " with name "
          + setRef->get_symbol ()->get_name ().getString (), 5);
    }

    if (calleeName.compare (OP2::OP_DECL_MAP_PREFIX) == 0)
    {
      /*
       * Add a new map associating the op_map variable reference with the arguments
       */
      SgVarRefExp * mapRef = isSgVarRefExp (args[OP2::OP_DECL_MAP_VAR_REF]);
      ROSE_ASSERT (mapRef != NULL);

      OP_MAP_Declarations.insert (pair <SgName, SgExpressionPtrList> (
          mapRef->get_symbol ()->get_name (), args));

      Debug::getInstance ()->debugMessage (OP2::OP_DECL_MAP_PREFIX
          + " call found: " + calleeName + " with name "
          + mapRef->get_symbol ()->get_name ().getString (), 5);
    }

    if (calleeName.compare (OP2::OP_DECL_DAT_PREFIX) == 0)
    {
      /*
       * Add a new map associating the OP_DAT variable reference with the arguments
       */
      SgVarRefExp * datRef = isSgVarRefExp (args[OP2::OP_DECL_DAT_VAR_REF]);
      ROSE_ASSERT (datRef != NULL);

      OP_DAT_Declarations.insert (pair <SgName, SgExpressionPtrList> (
          datRef->get_symbol ()->get_name (), args));

      Debug::getInstance ()->debugMessage (OP2::OP_DECL_DAT_PREFIX
          + " call found: " + calleeName + " with name "
          + datRef->get_symbol ()->get_name ().getString (), 5);
    }
  }
}
