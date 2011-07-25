#include <CPPParallelLoop.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <Globals.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPParallelLoop::handleOpGblDeclaration (OpGblDefinition * opGblDeclaration,
    std::string const & variableName, int opDatArgumentGroup)
{
}

void
CPPParallelLoop::handleOpDatDeclaration (OpDatDefinition * opDatDeclaration,
    std::string const & variableName, int opDatArgumentGroup)
{
}

void
CPPParallelLoop::retrieveOpDatDeclarations (
    CPPProgramDeclarationsAndDefinitions * declarations)
{
  using boost::iequals;
  using boost::lexical_cast;
  using std::find;
  using std::string;
  using std::vector;
  using std::map;

  Debug::getInstance ()->debugMessage ("Retrieving OP_DAT declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  int opDatArgumentGroup = 0;

  /*
   * ======================================================
   * The iterator starts from position NUMBER_OF_NON_OP_DAT_ARGUMENTS
   * to avoid the user subroutine name and the OP_SET
   * ======================================================
   */

  for (vector <SgExpression *>::iterator it = getActualArguments ().begin ()
      + NUMBER_OF_NON_OP_DAT_ARGUMENTS; it != getActualArguments ().end (); ++it)
  {
    std::cout << (*it)->class_name () << std::endl;

    switch ((*it)->variantT ())
    {
      case V_SgFunctionCallExp:
      {
        /*
         * ======================================================
         * The argument of the OP_PAR_LOOP is a variable
         * reference (expression)
         * ======================================================
         */

        SgFunctionCallExp * callExpression = isSgFunctionCallExp (*it);

        string const
            calleeName =
                callExpression->getAssociatedFunctionSymbol ()->get_name ().getString ();

        Debug::getInstance ()->debugMessage ("Found function call to '"
            + calleeName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

        if (iequals (calleeName, OP2::OP_ARG_DAT))
        {
        }
        else if (iequals (calleeName, OP2::OP_ARG_GBL))
        {
        }

        break;
      }

      default:
      {
        break;
      }
    }
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
CPPParallelLoop::generateReductionSubroutines (SgScopeStatement * moduleScope)
{
}

unsigned int
CPPParallelLoop::getNumberOfOpDatArgumentGroups ()
{
  return getActualArguments ().size () - NUMBER_OF_NON_OP_DAT_ARGUMENTS;
}

CPPParallelLoop::CPPParallelLoop (SgFunctionCallExp * functionCallExpression,
    std::string userSubroutineName,
    CPPProgramDeclarationsAndDefinitions * declarations) :
  ParallelLoop <SgFunctionDeclaration, CPPProgramDeclarationsAndDefinitions> (
      functionCallExpression)
{
  retrieveOpDatDeclarations (declarations);

  if (isDirectLoop ())
  {
    Debug::getInstance ()->debugMessage ("'" + userSubroutineName
        + "' is a DIRECT loop", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  }
  else
  {
    Debug::getInstance ()->debugMessage ("'" + userSubroutineName
        + "' is an INDIRECT loop", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  }
}
