#include <FortranParallelLoop.h>
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
FortranParallelLoop::handleOpGblDeclaration (
    OpGblDefinition * opGblDeclaration, std::string const & variableName,
    int opDatArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "'" + variableName
          + "' has been declared through OP_DECL_GBL (and not through OP_DECL_DAT)",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  uniqueOpDats.push_back (variableName);

  OpDatTypes[opDatArgumentGroup] = opGblDeclaration->getPrimitiveType ();

  OpDatDimensions[opDatArgumentGroup] = opGblDeclaration->getDimension ();

  OpDatVariableNames[opDatArgumentGroup] = variableName;

  OpDatDuplicates[opDatArgumentGroup] = false;
}

void
FortranParallelLoop::handleOpDatDeclaration (
    OpDatDefinition * opDatDeclaration, std::string const & variableName,
    int opDatArgumentGroup)
{
  using boost::lexical_cast;
  using std::find;
  using std::string;

  OpDatTypes[opDatArgumentGroup] = opDatDeclaration->getPrimitiveType ();

  OpDatDimensions[opDatArgumentGroup] = opDatDeclaration->getDimension ();

  OpDatVariableNames[opDatArgumentGroup] = variableName;

  if (find (uniqueOpDats.begin (), uniqueOpDats.end (), variableName)
      == uniqueOpDats.end ())
  {
    uniqueOpDats.push_back (variableName);

    OpDatDuplicates[opDatArgumentGroup] = false;

    SgArrayType * isArrayType = isSgArrayType (
        opDatDeclaration->getPrimitiveType ());

    if (isArrayType == NULL)
    {
      Debug::getInstance ()->errorMessage ("OP_DAT '" + variableName
          + "' is not an array");
    }

    SgType * baseType = isArrayType->get_base_type ();

    if (isSgTypeFloat (baseType) != NULL)
    {
      SgIntVal * sizeOfRealType = isSgIntVal (baseType->get_type_kind ());

      if (sizeOfRealType == NULL)
      {
        Debug::getInstance ()->errorMessage (
            "The size of the base type of OP_DAT '" + variableName
                + "' cannot be determined");
      }
      else
      {
        Debug::getInstance ()->debugMessage ("...size of the base type is "
            + lexical_cast <string> (sizeOfRealType->get_value ()),
            Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

        sizeOfOpDat = sizeOfRealType->get_value ();
      }
    }
  }
  else
  {
    OpDatDuplicates[opDatArgumentGroup] = true;
  }
}

void
FortranParallelLoop::retrieveOpDatDeclarations (
    FortranProgramDeclarationsAndDefinitions * declarations)
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
    switch ((*it)->variantT ())
    {
      case V_SgVarRefExp:
      {
        /*
         * ======================================================
         * The argument of the OP_PAR_LOOP is a variable
         * reference (expression)
         * ======================================================
         */

        SgVarRefExp * variableReference = isSgVarRefExp (*it);

        switch (variableReference->get_type ()->variantT ())
        {
          case V_SgClassType:
          {
            SgClassType * classReference = isSgClassType (
                variableReference->get_type ());

            string const variableName =
                variableReference->get_symbol ()->get_name ().getString ();

            string const className = classReference->get_name ().getString ();

            if (iequals (className, OP2::OP_DAT))
            {
              opDatArgumentGroup++;

              /*
               * ======================================================
               * Found an OP_DAT variable. To retrieve its declaration,
               * we must check whether it was declared via OP_DECL_DAT
               * or OP_DECL_GBL
               * ======================================================
               */

              try
              {
                handleOpDatDeclaration (declarations->getOpDatDefinition (
                    variableName), variableName, opDatArgumentGroup);
              }
              catch (const std::string &)
              {
                handleOpGblDeclaration (declarations->getOpGblDefinition (
                    variableName), variableName, opDatArgumentGroup);
              }
            }

            else if (iequals (className, OP2::OP_MAP))
            {
              if (iequals (variableName, OP2::OP_ID))
              {
                /*
                 * ======================================================
                 * OP_ID signals identity mapping and therefore direct
                 * access to the data
                 * ======================================================
                 */
                Debug::getInstance ()->debugMessage (
                    "...DIRECT mapping descriptor", Debug::OUTER_LOOP_LEVEL,
                    __FILE__, __LINE__);

                OpDatMappingDescriptors[opDatArgumentGroup] = DIRECT;
              }
              else
              {
                if (iequals (variableName, OP2::OP_GBL))
                {
                  /*
                   * ======================================================
                   * OP_GBL signals that the OP_DAT is a global variable
                   * ======================================================
                   */
                  Debug::getInstance ()->debugMessage (
                      "...GLOBAL mapping descriptor", Debug::OUTER_LOOP_LEVEL,
                      __FILE__, __LINE__);

                  OpDatMappingDescriptors[opDatArgumentGroup] = GLOBAL;
                }

                else
                {
                  Debug::getInstance ()->debugMessage (
                      "...INDIRECT mapping descriptor",
                      Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

                  OpDatMappingDescriptors[opDatArgumentGroup] = INDIRECT;
                }
              }
            }

            break;
          }

          case V_SgTypeInt:
          {
            string const variableName =
                variableReference->get_symbol ()->get_name ().getString ();

            if (iequals (variableName, OP2::OP_READ))
            {
              Debug::getInstance ()->debugMessage ("...READ access descriptor",
                  Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

              OpDatAccessDescriptors[opDatArgumentGroup] = READ_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_WRITE))
            {
              Debug::getInstance ()->debugMessage (
                  "...WRITE access descriptor", Debug::OUTER_LOOP_LEVEL,
                  __FILE__, __LINE__);

              OpDatAccessDescriptors[opDatArgumentGroup] = WRITE_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_INC))
            {
              Debug::getInstance ()->debugMessage (
                  "...INCREMENT access descriptor", Debug::OUTER_LOOP_LEVEL,
                  __FILE__, __LINE__);

              OpDatAccessDescriptors[opDatArgumentGroup] = INC_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_RW))
            {
              Debug::getInstance ()->debugMessage (
                  "...READ/WRITE access descriptor", Debug::OUTER_LOOP_LEVEL,
                  __FILE__, __LINE__);

              OpDatAccessDescriptors[opDatArgumentGroup] = RW_ACCESS;
            }

            else
            {
              Debug::getInstance ()->errorMessage (
                  "Unknown access descriptor: '" + variableName
                      + "' for OP_DAT argument #" + lexical_cast <string> (
                      opDatArgumentGroup));
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
}

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
  return (getActualArguments().size () - NUMBER_OF_NON_OP_DAT_ARGUMENTS)
      / NUMBER_OF_ARGUMENTS_PER_OP_DAT;
}

FortranParallelLoop::FortranParallelLoop (
    SgFunctionCallExp * functionCallExpression, std::string userSubroutineName,
    FortranProgramDeclarationsAndDefinitions * declarations) :
  ParallelLoop <SgProcedureHeaderStatement,
      FortranProgramDeclarationsAndDefinitions> (functionCallExpression)
{
  using boost::iequals;

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
