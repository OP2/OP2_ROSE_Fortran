#include <boost/algorithm/string.hpp>

#include "Debug.h"
#include "OP2ParallelLoop.h"

OP2ParallelLoop::OP2ParallelLoop (std::string userSubroutineName,
    SgExpressionPtrList & actualArguments,
    Declarations * op2DeclaredVariables)
{
  this->CUDAModuleName = userSubroutineName + "_cudafor";
  this->actualArguments = actualArguments;

  setDirectOrIndirectLoop (op2DeclaredVariables);
  retrieve_OP_DAT_Declarations (op2DeclaredVariables);

  if (isDirect)
  {
    hostSubroutine = new HostSubroutineOfDirectLoop (userSubroutineName);
  }
  else
  {
    hostSubroutine = new HostSubroutineOfIndirectLoop (userSubroutineName);
  }

  kernelSubroutine = new KernelSubroutine (userSubroutineName);
  userDeviceSubroutine = new UserDeviceSubroutine (userSubroutineName);
  userHostSubroutine = new UserHostSubroutine (userSubroutineName);

  if (isDirect)
  {
    Debug::getInstance ()->debugMessage ("'" + userSubroutineName
        + "' is a DIRECT loop", 5);
  }
  else
  {
    Debug::getInstance ()->debugMessage ("'" + userSubroutineName
        + "' is an INDIRECT loop", 5);
  }
}

void
OP2ParallelLoop::setDirectOrIndirectLoop (
    Declarations * op2DeclaredVariables)
{
  using boost::iequals;
  using std::string;
  using std::vector;

  /*
   * ======================================================
   * Initially assume that the OP_PAR_LOOP is direct
   * ======================================================
   */
  isDirect = true;

  for (vector <SgExpression *>::iterator it = actualArguments.begin ()
      + OP2::NUMBER_OF_NON_OP_DAT_ARGUMENTS; it != actualArguments.end (); ++it)
  {
    if ((*it)->variantT () == V_SgVarRefExp)
    {
      /*
       * ======================================================
       * The argument of the OP_PAR_LOOP is a variable
       * reference (expression)
       * ======================================================
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
             * ======================================================
             * OP_ID signals identity mapping and therefore direct
             * access to the data. If we discover an OP_MAP that is
             * not an OP_ID then an indirect loop is instead assumed
             * ======================================================
             */
            isDirect = false;
          }
        }
      }
    }
  }
}

void
OP2ParallelLoop::retrieve_OP_DAT_Declarations (
    Declarations * op2DeclaredVariables)
{
  using boost::iequals;
  using std::string;
  using std::vector;
  using std::map;
  using std::make_pair;

  Debug::getInstance ()->debugMessage ("Retrieving OP_DAT declarations", 2);

  for (vector <SgExpression *>::iterator it = actualArguments.begin ()
      + OP2::NUMBER_OF_NON_OP_DAT_ARGUMENTS; it != actualArguments.end (); ++it)
  {
    if ((*it)->variantT () == V_SgVarRefExp)
    {
      /*
       * ======================================================
       * The argument of the OP_PAR_LOOP is a variable
       * reference (expression)
       * ======================================================
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
           * ======================================================
           * Found an OP_DAT variable
           *
           * TODO: this needs to be fixed. OP_DAT variables can be
           * declared in 2 ways:
           * 1) Through OP_DECL_DAT
           * 2) Through OP_DECL_GBL
           *
           * Therefore, we might NOT find the OP_DAT from the OP_DAT
           * declarations, but instead find it in the OP_GBL declarations
           * ======================================================
           */
          try
          {
            OP_DAT_Declaration * opDatDeclaration =
                op2DeclaredVariables->get_OP_DAT_Declaration (variableName);

            if (OP_DATs.count (variableName) == 0)
            {
              OP_DATs.insert (make_pair (variableName, opDatDeclaration));
              OP_DAT_Occurrences.insert (make_pair (variableName, 1));
            }
            else
            {
              unsigned int occurrences = OP_DAT_Occurrences[variableName] + 1;
              OP_DAT_Occurrences.insert (make_pair (variableName, occurrences));
            }
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
