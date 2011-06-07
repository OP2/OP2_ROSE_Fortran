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
ParallelLoop::retrieveOP_DATDeclarations (Declarations * declarations)
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

                OP_DAT_Types[OP_DATCounter]
                    = opDatDeclaration->getActualType ();

                OP_DAT_Dimensions[OP_DATCounter]
                    = opDatDeclaration->getDimension ();

                OP_DAT_VariableNames[OP_DATCounter] = variableName;

                if (find (unique_OP_DATs.begin (), unique_OP_DATs.end (),
                    variableName) == unique_OP_DATs.end ())
                {
                  unique_OP_DATs.push_back (variableName);

                  OP_DAT_Duplicates[OP_DATCounter] = false;

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

                      sizeOf_OP_DAT = sizeOfRealType->get_value ();
                    }
                  }
                }
                else
                {
                  OP_DAT_Duplicates[OP_DATCounter] = true;
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

                OP_DAT_Types[OP_DATCounter]
                    = opGBLDeclaration->getActualType ();

                OP_DAT_Dimensions[OP_DATCounter]
                    = opGBLDeclaration->getDimension ();

                OP_DAT_VariableNames[OP_DATCounter] = variableName;

                unique_OP_DATs.push_back (variableName);

                OP_DAT_Duplicates[OP_DATCounter] = false;
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
                OP_DAT_MappingDescriptors[OP_DATCounter] = DIRECT;
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

                  OP_DAT_MappingDescriptors[OP_DATCounter] = GLOBAL;
                }

                else
                {
                  OP_DAT_MappingDescriptors[OP_DATCounter] = INDIRECT;
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
              OP_DAT_AccessDescriptors[OP_DATCounter] = READ_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_WRITE))
            {
              OP_DAT_AccessDescriptors[OP_DATCounter] = WRITE_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_INC))
            {
              OP_DAT_AccessDescriptors[OP_DATCounter] = INC_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_RW))
            {
              OP_DAT_AccessDescriptors[OP_DATCounter] = RW_ACCESS;
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
    for (unsigned int i = 1; i <= getNumberOf_OP_DAT_ArgumentGroups (); ++i)
    {
      if (isReductionRequired (i) == true)
      {
        SgType * opDatType = get_OP_DAT_Type (i);

        SgArrayType * isArrayType = isSgArrayType (opDatType);

        ROSE_ASSERT ( isArrayType != NULL );

        SgExpression * opDatKindSize =
            FortranStatementsAndExpressionsBuilder::getFortranKindOf_OP_DAT (
                isArrayType);

        SgIntVal * isKindIntVal = isSgIntVal (opDatKindSize);

        ROSE_ASSERT ( isKindIntVal != NULL );

        string typeName;

        if (isSgTypeInt (isArrayType->get_base_type ()) != NULL)
        {
          typeName = SubroutineNameSuffixes::integerSuffix;
        }
        else if (isSgTypeFloat (isArrayType->get_base_type ()) != NULL)
        {
          typeName = SubroutineNameSuffixes::floatSuffix;
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
            + lexical_cast <string> (i)
            + SubroutineNameSuffixes::reductionSuffix + typeName
            + lexical_cast <string> (isKindIntVal->get_value ());

        FortranCUDAReductionSubroutine * reductionSubroutine =
            new FortranCUDAReductionSubroutine (reductionSubroutineName,
                moduleScope, isArrayType);

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
      OP_DAT_MappingDescriptors.begin (); it
      != OP_DAT_MappingDescriptors.end (); ++it)
  {
    if (it->second == INDIRECT)
    {
      return false;
    }
  }

  return true;
}

unsigned int
ParallelLoop::getNumberOfDistinctIndirect_OP_DAT_Arguments ()
{
  using std::map;

  unsigned int count = 0;

  for (unsigned int i = 1; i <= getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (OP_DAT_Duplicates[i] == false)
    {
      if (OP_DAT_MappingDescriptors[i] == INDIRECT)
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
ParallelLoop::getNumberOf_OP_DAT_ArgumentGroups () const
{
  return (actualArguments.size ()
      - OP2::Fortran::NUMBER_OF_NON_OP_DAT_ARGUMENTS)
      / OP2::Fortran::NUMBER_OF_ARGUMENTS_PER_OP_DAT;
}

SgType *
ParallelLoop::get_OP_DAT_Type (unsigned int OP_DAT_ArgumentGroup)
{
  return OP_DAT_Types[OP_DAT_ArgumentGroup];
}

unsigned int
ParallelLoop::get_OP_DAT_Dimension (unsigned int OP_DAT_ArgumentGroup)
{
  return OP_DAT_Dimensions[OP_DAT_ArgumentGroup];
}

bool
ParallelLoop::isDuplicate_OP_DAT (unsigned int OP_DAT_ArgumentGroup)
{
  return OP_DAT_Duplicates[OP_DAT_ArgumentGroup];
}

MAPPING_VALUE
ParallelLoop::get_OP_MAP_Value (unsigned int OP_DAT_ArgumentGroup)
{
  return OP_DAT_MappingDescriptors[OP_DAT_ArgumentGroup];
}

ACCESS_CODE_VALUE
ParallelLoop::get_OP_Access_Value (unsigned int OP_DAT_ArgumentGroup)
{
  return OP_DAT_AccessDescriptors[OP_DAT_ArgumentGroup];
}

unsigned int
ParallelLoop::getNumberOfIndirectDataSets ()
{
  int count = 0;

  for (unsigned int i = 1; i <= getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (OP_DAT_MappingDescriptors[i] == INDIRECT)
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

  for (unsigned int i = 1; i <= getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (OP_DAT_MappingDescriptors[i] == INDIRECT && isDuplicate_OP_DAT (i)
        == false)
    {
      count++;
    }
  }

  return count;
}

bool
ParallelLoop::isReductionRequired ()
{
  for (unsigned int i = 1; i <= getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (OP_DAT_MappingDescriptors[i] == GLOBAL && OP_DAT_AccessDescriptors[i]
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
  return OP_DAT_MappingDescriptors[OP_DAT_ArgumentGroup] == GLOBAL
      && OP_DAT_AccessDescriptors[OP_DAT_ArgumentGroup] != READ_ACCESS;
}

std::string
ParallelLoop::get_OP_DAT_VariableName (unsigned int OP_DAT_ArgumentGroup)
{
  return OP_DAT_VariableNames[OP_DAT_ArgumentGroup];
}

unsigned int
ParallelLoop::getSizeOf_OP_DAT () const
{
  return sizeOf_OP_DAT;
}

SgProcedureHeaderStatement *
ParallelLoop::getReductionSubroutineHeader (unsigned int OP_DAT_ArgumentGroup)
{
  return reductionSubroutines[OP_DAT_ArgumentGroup];
}

ParallelLoop::ParallelLoop (std::string userSubroutineName,
    SgExpressionPtrList & actualArguments, Declarations * declarations)
{
  using boost::iequals;

  this->actualArguments = actualArguments;

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

  retrieveOP_DATDeclarations (declarations);

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
