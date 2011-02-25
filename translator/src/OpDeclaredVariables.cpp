#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include "OpDeclaredVariables.h"
#include "OP2CommonDefinitions.h"
#include "Debug.h"

void
OpDeclaredVariables::visit (SgNode * node)
{
  using namespace boost;
  using namespace std;
  using namespace OP2;

  switch (node->variantT ())
  {
    case V_SgFunctionCallExp:
    {
      /*
       * Function call found in the AST
       */
      SgFunctionCallExp* functionCallExp = isSgFunctionCallExp (node);

      /*
       * Retrieve the arguments of the call
       */
      SgExpressionPtrList & args =
          functionCallExp->get_args ()->get_expressions ();

      /*
       * The name of the called function
       */
      string const
          calleeName =
              functionCallExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();

      if (starts_with (calleeName, OP_DECL_SET_PREFIX))
      {
        /*
         * Add a new map associating the op_set variable reference with the arguments
         */
        SgVarRefExp * setRef = isSgVarRefExp (args[OP_DECL_SET_VAR_REF]);
        ROSE_ASSERT(setRef!=NULL);

        declaredOpSets.insert (pair <SgName, SgExpressionPtrList> (
            setRef->get_symbol ()->get_name (), args));

        Debug::getInstance ()->debugMessage ("OP_DECL_SET call found: "
            + calleeName + " with name "
            + setRef->get_symbol ()->get_name ().getString (), 5);
      }

      if (starts_with (calleeName, OP_DECL_MAP_PREFIX))
      {
        /*
         * Add a new map associating the op_map variable reference with the arguments
         */
        SgVarRefExp * mapRef = isSgVarRefExp (args[OP_DECL_MAP_VAR_REF]);
        ROSE_ASSERT(mapRef!=NULL);

        declaredOpMaps.insert (pair <SgName, SgExpressionPtrList> (
            mapRef->get_symbol ()->get_name (), args));

        Debug::getInstance ()->debugMessage ("OP_DECL_MAP call found: "
            + calleeName + " with name "
            + mapRef->get_symbol ()->get_name ().getString (), 5);
      }

      if (starts_with (calleeName, OP_DECL_DAT_PREFIX))
      {
        /*
         * Add a new map associating the op_dat variable reference with the arguments
         */
        SgVarRefExp * datRef = isSgVarRefExp (args[OP_DECL_DAT_VAR_REF]);
        ROSE_ASSERT(datRef!=NULL);

        declaredOpDats.insert (pair <SgName, SgExpressionPtrList> (
            datRef->get_symbol ()->get_name (), args));

        Debug::getInstance ()->debugMessage ("OP_DECL_DAT call found: "
            + calleeName + " with name "
            + datRef->get_symbol ()->get_name ().getString (), 5);
      }

      break;
    }

    case V_SgSourceFile:
    {
      SgSourceFile * sourceFile = isSgSourceFile (node);

      Debug::getInstance ()->debugMessage ("Found file "
          + sourceFile->getFileName (), 5);
      break;
    }

    default:
    {
      break;
    }
  }
}
