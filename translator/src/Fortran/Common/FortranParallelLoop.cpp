#include <FortranParallelLoop.h>
#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
FortranParallelLoop::generateReductionSubroutines (
    SgScopeStatement * moduleScope)
{
  using boost::lexical_cast;
  using std::string;
  using std::map;

  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (isReductionRequired (i) == true)
    {
      SgArrayType * arrayType = isSgArrayType (getOpDatType (i));

      SgExpression * opDatKindSize =
          FortranStatementsAndExpressionsBuilder::getFortranKindOfOpDat (
              arrayType);

      SgIntVal * isKindIntVal = isSgIntVal (opDatKindSize);

      ROSE_ASSERT (isKindIntVal != NULL);

      string typeName;

      if (isSgTypeInt (arrayType->get_base_type ()) != NULL)
      {
        typeName = "_integer";
      }
      else if (isSgTypeFloat (arrayType->get_base_type ()) != NULL)
      {
        typeName = "_float";
      }
      else
      {
        Debug::getInstance ()->errorMessage (
            "Error: type for reduction variable is not supported");
      }

      /*
       * ======================================================
       * For now we distinguish between subroutines by also
       * appending the index of the related OP_DAT argument.
       * Eventually, the factorisation will solve this problem
       * ======================================================
       */
      string const reductionSubroutineName = "arg" + lexical_cast <string> (i)
          + "_reduction" + typeName + lexical_cast <string> (
          isKindIntVal->get_value ());

      FortranCUDAReductionSubroutine * reductionSubroutine =
          new FortranCUDAReductionSubroutine (reductionSubroutineName,
              moduleScope, arrayType);

      /*
       * ======================================================
       * Generate one per reduction variable, eventually
       * we will have to factorise
       * ======================================================
       */
      reductionSubroutines[i]
          = reductionSubroutine->getSubroutineHeaderStatement ();
    }
  }
}

unsigned int
FortranParallelLoop::getNumberOfOpDatArgumentGroups ()
{
  return (getActualArguments ().size () - NUMBER_OF_NON_OP_DAT_ARGUMENTS)
      / NUMBER_OF_ARGUMENTS_PER_OP_DAT;
}

FortranParallelLoop::FortranParallelLoop (
    SgFunctionCallExp * functionCallExpression) :
  ParallelLoop <SgProcedureHeaderStatement> (functionCallExpression)
{
}
