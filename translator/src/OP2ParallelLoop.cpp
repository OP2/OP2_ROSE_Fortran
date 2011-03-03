#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Debug.h"
#include "OP2ParallelLoop.h"

void
OP2ParallelLoop::retrieveArgumentTypes (
    OP2DeclaredVariables * op2DeclaredVariables)
{
  using boost::lexical_cast;
  using boost::iequals;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Retrieving actual types of OP_DAT arguments", 2);

  /*
   * Scan OP_DAT arguments and get their actual base types.
   * Also determine whether this is a direct/indirect loop by inspecting OP_MAP arguments
   * Note that the first batch of OP_DAT arguments starts at index NUMBER_OF_NON_OP_DAT_ARGUMENTS.
   */
  for (vector <SgExpression *>::iterator it = actualArguments.begin ()
      + OP2::NUMBER_OF_NON_OP_DAT_ARGUMENTS; it != actualArguments.end (); ++it)
  {
    if ((*it)->variantT () == V_SgVarRefExp)
    {
      /*
       * The argument of the OP_PAR_LOOP is a variable reference (expression)
       */
      SgVarRefExp * variableReference = isSgVarRefExp (*it);

      if (variableReference->get_type ()->variantT () == V_SgClassType)
      {
        SgClassType* classReference = isSgClassType (
            variableReference->get_type ());
        ROSE_ASSERT (classReference != NULL);

        string const variableName =
            variableReference->get_symbol ()->get_name ().getString ();

        string const className = classReference->get_name ().getString ();

        if (iequals (className, OP2::OP_DAT_NAME))
        {
          /*
           * Found an OP_DAT variable, so retrieve its dimensions and its
           * type from the arguments passed to OP_DECL_DAT
           */
          SgExpressionPtrList OP_DECL_DAT_Arguments =
              op2DeclaredVariables->get_OP_DECL_DAT_Arguments (variableName);

          if (OP_DECL_DAT_Arguments.size () > 0)
          {
            /*
             * TODO: this needs to be fixed. OP_DAT variables can be declared in 2 ways:
             * 1) Through OP_DECL_DAT
             * 2) Through OP_DECL_GBL
             * An OP_DECL_GBL has 3 parameters, whereas an OP_DECL_DAT has 4 parameters
             */
            SgIntVal
                * opDatDimension =
                    isSgIntVal (
                        OP_DECL_DAT_Arguments[OP2::INDEX_OF_DIMENSION_IN_OP_DECL_DAT_PARAMETER_LIST]);
            ROSE_ASSERT (opDatDimension != NULL);
            set_OP_DAT_Dimension (opDatDimension->get_value ());

            SgVarRefExp
                * opDatInputArg =
                    isSgVarRefExp (
                        OP_DECL_DAT_Arguments[OP2::INDEX_OF_DATA_TYPE_IN_OP_DECL_DAT_PARAMETER_LIST]);
            ROSE_ASSERT (opDatInputArg != NULL);
            set_OP_DAT_ActualType (opDatInputArg->get_type ());

            Debug::getInstance ()->debugMessage ("The OP_DAT variable '"
                + variableName + "' has actual type "
                + opDatInputArg->get_type ()->class_name () + " and "
                + lexical_cast <string> (opDatDimension->get_value ())
                + " dimensions", 6);
          }
          else
          {
          }
        }
        else if (iequals (className, OP2::OP_MAP_NAME))
        {
          if (iequals (variableName, OP2::OP_ID_NAME) == false)
          {
            /*
             * OP_ID signals identity mapping and therefore direct access to the data.
             * If we discover an OP_MAP that is not an OP_ID then an indirect loop
             * is instead assumed
             */
            isDirect = false;
          }
        }
      }
    }
  }

  if (isDirect)
  {
    Debug::getInstance ()->debugMessage ("'" + kernelHostName
        + "' is a DIRECT loop", 5);
  }
  else
  {
    Debug::getInstance ()->debugMessage ("'" + kernelHostName
        + "' is an INDIRECT loop", 5);
  }
}
