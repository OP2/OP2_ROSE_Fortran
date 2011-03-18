#include <boost/algorithm/string.hpp>

#include "Debug.h"
#include "OP2ParallelLoop.h"

void
OP2ParallelLoop::setDirectOrIndirectLoop (
    OP2DeclaredVariables * op2DeclaredVariables)
{
  using boost::iequals;
  using std::string;
  using std::vector;

  /*
   * Initially assume that the OP_PAR_LOOP is direct
   */
  isDirect = true;

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
        SgClassType * classReference = isSgClassType (
            variableReference->get_type ());

        string const variableName =
            variableReference->get_symbol ()->get_name ().getString ();

        string const className = classReference->get_name ().getString ();

        if (iequals (className, OP2::OP_MAP_NAME))
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
    Debug::getInstance ()->debugMessage ("'" + userHostFunctionName
        + "' is a DIRECT loop", 5);
  }
  else
  {
    Debug::getInstance ()->debugMessage ("'" + userHostFunctionName
        + "' is an INDIRECT loop", 5);
  }
}

void
OP2ParallelLoop::retrieve_OP_DAT_BaseTypes (
    OP2DeclaredVariables * op2DeclaredVariables)
{
  using boost::iequals;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Retrieving base types of OP_DAT arguments", 2);

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
        SgClassType * classReference = isSgClassType (
            variableReference->get_type ());

        string const variableName =
            variableReference->get_symbol ()->get_name ().getString ();

        string const className = classReference->get_name ().getString ();

        if (iequals (className, OP2::OP_DAT_NAME))
        {
          /*
           * Found an OP_DAT variable, so retrieve its dimensions and its
           * type from the arguments passed to OP_DECL_D
           *
           * TODO: this needs to be fixed. OP_DAT variables can be declared in 2 ways:
           * 1) Through OP_DECL_DAT
           * 2) Through OP_DECL_GBL
           * Therefore, we might NOT find the OP_DAT from the OP_DAT declarations, but
           * instead find it in the OP_GBL declarations
           */
          try
          {
            OP_DAT_Declaration * opDatDeclaration =
                op2DeclaredVariables->get_OP_DAT_Declaration (variableName);

            OP_DAT_Dimensions.push_back (opDatDeclaration->getDimension ());
            OP_DAT_ActualTypes.push_back (opDatDeclaration->getActualType ());
          }
          catch (std::string const & variableName)
          {
            Debug::getInstance ()->debugMessage (
                "'" + variableName
                    + "' has not been declared through OP_DECL_DAT. It must have been declared using OP_DECL_GBL",
                1);
          }
        }
      }
    }
  }
}
