#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <Debug.h>
#include <ParallelLoop.h>
#include <FortranCUDAHostSubroutineDirectLoop.h>
#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <Globals.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
ParallelLoop::retrieveOpDatDeclarations (Declarations * declarations)
{
  using boost::iequals;
  using boost::lexical_cast;
  using std::find;
  using std::string;
  using std::vector;
  using std::map;

  Debug::getInstance ()->debugMessage ("Retrieving OP_DAT declarations", 2);

  int OP_DATCounter = 0;

  /*
   * ======================================================
   * The iterator starts from position NUMBER_OF_NON_OP_DAT_ARGUMENTS
   * to avoid the user subroutine name and the OP_SET
   * ======================================================
   */

  for (vector <SgExpression *>::iterator it = actualArguments.begin ()
      + OP2::Fortran::NUMBER_OF_NON_OP_DAT_ARGUMENTS; it
      != actualArguments.end (); ++it)
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
              OP_DATCounter++;

              /*
               * ======================================================
               * Found an OP_DAT variable. To retrieve its declaration,
               * we must check whether it was declared via OP_DECL_DAT
               * or OP_DECL_GBL
               * ======================================================
               */

              try
              {
                OP_DAT_Declaration * opDatDeclaration =
                    declarations->get_OP_DAT_Declaration (variableName);

                OpDatTypes[OP_DATCounter] = opDatDeclaration->getActualType ();

                OpDatDimensions[OP_DATCounter]
                    = opDatDeclaration->getDimension ();

                OpDatVariableNames[OP_DATCounter] = variableName;

                if (find (uniqueOpDats.begin (), uniqueOpDats.end (),
                    variableName) == uniqueOpDats.end ())
                {
                  uniqueOpDats.push_back (variableName);

                  OpDatDuplicates[OP_DATCounter] = false;

                  SgArrayType * isArrayType = isSgArrayType (
                      opDatDeclaration->getActualType ());

                  if (isArrayType == NULL)
                  {
                    Debug::getInstance ()->errorMessage ("OP_DAT '"
                        + variableName + "' is not an array");
                  }

                  SgType * baseType = isArrayType->get_base_type ();

                  if (isSgTypeFloat (baseType) != NULL)
                  {
                    SgIntVal * sizeOfRealType = isSgIntVal (
                        baseType->get_type_kind ());

                    if (sizeOfRealType == NULL)
                    {
                      Debug::getInstance ()->errorMessage (
                          "The size of the base type of OP_DAT '"
                              + variableName + "' cannot be determined");
                    }
                    else
                    {
                      Debug::getInstance ()->debugMessage (
                          "The size of the base type of OP_DAT '"
                              + variableName + "' is "
                              + lexical_cast <string> (
                                  sizeOfRealType->get_value ()), 7);

                      sizeOfOpDat = sizeOfRealType->get_value ();
                    }
                  }
                }
                else
                {
                  OpDatDuplicates[OP_DATCounter] = true;
                }
              }
              catch (const std::string &)
              {
                Debug::getInstance ()->debugMessage (
                    "'" + variableName
                        + "' has been declared through OP_DECL_GBL (and not through OP_DECL_DAT)",
                    1);

                OP_GBL_Declaration * opGBLDeclaration =
                    declarations->get_OP_GBL_Declaration (variableName);

                OpDatTypes[OP_DATCounter] = opGBLDeclaration->getActualType ();

                OpDatDimensions[OP_DATCounter]
                    = opGBLDeclaration->getDimension ();

                OpDatVariableNames[OP_DATCounter] = variableName;

                uniqueOpDats.push_back (variableName);

                OpDatDuplicates[OP_DATCounter] = false;
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
                OpDatMappingDescriptors[OP_DATCounter] = DIRECT;
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

                  OpDatMappingDescriptors[OP_DATCounter] = GLOBAL;
                }

                else
                {
                  OpDatMappingDescriptors[OP_DATCounter] = INDIRECT;
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
              OpDatAccessDescriptors[OP_DATCounter] = READ_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_WRITE))
            {
              OpDatAccessDescriptors[OP_DATCounter] = WRITE_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_INC))
            {
              OpDatAccessDescriptors[OP_DATCounter] = INC_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_RW))
            {
              OpDatAccessDescriptors[OP_DATCounter] = RW_ACCESS;
            }

            else
            {
              Debug::getInstance ()->errorMessage (
                  "Unknown access descriptor: '" + variableName
                      + "' for OP_DAT argument #" + lexical_cast <string> (
                      OP_DATCounter));
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
ParallelLoop::generateReductionSubroutines (SgScopeStatement * moduleScope)
{
  using boost::lexical_cast;
  using std::string;
  using std::map;

  if (isReductionRequired () == true)
  {
    for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
    {
      if (isReductionRequired (i) == true)
      {
        SgArrayType * arrayType = isSgArrayType (getOpDatType (i));

        SgExpression * opDatKindSize =
            FortranStatementsAndExpressionsBuilder::getFortranKindOf_OP_DAT (
                arrayType);

        SgIntVal * isKindIntVal = isSgIntVal (opDatKindSize);

        ROSE_ASSERT ( isKindIntVal != NULL );

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
        string const reductionSubroutineName = "arg"
            + lexical_cast <string> (i) + "_reduction" + typeName
            + lexical_cast <string> (isKindIntVal->get_value ());

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
            = reductionSubroutine->getProcedureHeaderStatement ();
      }
    }
  }
}

bool
ParallelLoop::isDirectLoop () const
{
  using std::map;

  for (map <unsigned int, MAPPING_VALUE>::const_iterator it =
      OpDatMappingDescriptors.begin (); it != OpDatMappingDescriptors.end (); ++it)
  {
    if (it->second == INDIRECT)
    {
      return false;
    }
  }

  return true;
}

unsigned int
ParallelLoop::getNumberOfDistinctIndirectOpDatArguments ()
{
  using std::map;

  unsigned int count = 0;

  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (OpDatDuplicates[i] == false)
    {
      if (OpDatMappingDescriptors[i] == INDIRECT)
      {
        count++;
      }
    }
  }

  return count;
}

std::string
ParallelLoop::getModuleName () const
{
  return moduleName;
}

SgExpressionPtrList &
ParallelLoop::getActualArguments ()
{
  return actualArguments;
}

unsigned int
ParallelLoop::getNumberOfOpDatArgumentGroups () const
{
  return (actualArguments.size ()
      - OP2::Fortran::NUMBER_OF_NON_OP_DAT_ARGUMENTS)
      / OP2::Fortran::NUMBER_OF_ARGUMENTS_PER_OP_DAT;
}

SgType *
ParallelLoop::getOpDatType (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatTypes[OP_DAT_ArgumentGroup];
}

unsigned int
ParallelLoop::getOpDatDimension (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatDimensions[OP_DAT_ArgumentGroup];
}

bool
ParallelLoop::isDuplicateOpDat (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatDuplicates[OP_DAT_ArgumentGroup];
}

MAPPING_VALUE
ParallelLoop::getOpMapValue (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatMappingDescriptors[OP_DAT_ArgumentGroup];
}

ACCESS_CODE_VALUE
ParallelLoop::getOpAccessValue (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatAccessDescriptors[OP_DAT_ArgumentGroup];
}

unsigned int
ParallelLoop::getNumberOfIndirectDataSets ()
{
  int count = 0;

  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (OpDatMappingDescriptors[i] == INDIRECT)
    {
      count++;
    }
  }

  return count;
}

unsigned int
ParallelLoop::getNumberOfDifferentIndirectDataSets ()
{
  int count = 0;

  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (OpDatMappingDescriptors[i] == INDIRECT && isDuplicateOpDat (i) == false)
    {
      count++;
    }
  }

  return count;
}

bool
ParallelLoop::isReductionRequired ()
{
  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (OpDatMappingDescriptors[i] == GLOBAL && OpDatAccessDescriptors[i]
        != READ_ACCESS)
    {
      return true;
    }
  }
  return false;
}

bool
ParallelLoop::isReductionRequired (int OP_DAT_ArgumentGroup)
{
  return OpDatMappingDescriptors[OP_DAT_ArgumentGroup] == GLOBAL
      && OpDatAccessDescriptors[OP_DAT_ArgumentGroup] != READ_ACCESS;
}

std::string
ParallelLoop::getOpDatVariableName (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatVariableNames[OP_DAT_ArgumentGroup];
}

unsigned int
ParallelLoop::getSizeOfOpDat () const
{
  return sizeOfOpDat;
}

SgProcedureHeaderStatement *
ParallelLoop::getReductionSubroutineHeader (unsigned int OP_DAT_ArgumentGroup)
{
  return reductionSubroutines[OP_DAT_ArgumentGroup];
}

ParallelLoop::ParallelLoop (std::string userSubroutineName,
    SgExpressionPtrList & actualArguments, Declarations * declarations) :
  actualArguments (actualArguments)
{
  using boost::iequals;

  if (iequals (Globals::getInstance ()->getTargetBackend (),
      TargetBackends::CUDA))
  {
    moduleName = userSubroutineName + "_cudafor";
  }
  else if (iequals (Globals::getInstance ()->getTargetBackend (),
      TargetBackends::OpenMP))
  {
    moduleName = userSubroutineName + "_openmp";
  }

  retrieveOpDatDeclarations (declarations);

  if (isDirectLoop ())
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
