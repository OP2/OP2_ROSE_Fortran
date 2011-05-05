#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <Debug.h>
#include <ParallelLoop.h>
#include <HostSubroutineOfDirectLoop.h>
#include <HostSubroutineOfIndirectLoop.h>

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
  using std::make_pair;

  Debug::getInstance ()->debugMessage ("Retrieving OP_DAT declarations", 2);

  int OP_DATCounter = 0;

  /*
   * ======================================================
   * The iterator starts from position NUMBER_OF_NON_OP_DAT_ARGUMENTS
   * to avoid the user subroutine name and the OP_SET
   * ======================================================
   */

  for (vector <SgExpression *>::iterator it = actualArguments.begin ()
      + OP2::NUMBER_OF_NON_OP_DAT_ARGUMENTS; it != actualArguments.end (); ++it)
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

            if (iequals (className, OP2::OP_DAT_NAME))
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

                OP_DAT_Types.insert (make_pair (OP_DATCounter,
                    opDatDeclaration->getActualType ()));

                OP_DAT_Dimensions.insert (make_pair (OP_DATCounter,
                    opDatDeclaration->getDimension ()));

                OP_DAT_VariableNames[OP_DATCounter] = variableName;

                if (find (unique_OP_DATs.begin (), unique_OP_DATs.end (),
                    variableName) == unique_OP_DATs.end ())
                {
                  unique_OP_DATs.push_back (variableName);

                  OP_DAT_Duplicates.insert (make_pair (OP_DATCounter, false));
                }
                else
                {
                  OP_DAT_Duplicates.insert (make_pair (OP_DATCounter, true));
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

                OP_DAT_Types.insert (make_pair (OP_DATCounter,
                    opGBLDeclaration->getActualType ()));

                OP_DAT_Dimensions.insert (make_pair (OP_DATCounter,
                    opGBLDeclaration->getDimension ()));

                OP_DAT_VariableNames[OP_DATCounter] = variableName;

                unique_OP_DATs.push_back (variableName);

                OP_DAT_Duplicates.insert (make_pair (OP_DATCounter, false));
              }
            }

            else if (iequals (className, OP2::OP_MAP_NAME))
            {
              if (iequals (variableName, OP2::OP_ID_NAME))
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
                if (iequals (variableName, OP2::OP_GBL_NAME))
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

            if (iequals (variableName, OP2::OP_READ_NAME))
            {
              OP_DAT_AccessDescriptors[OP_DATCounter] = READ_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_WRITE_NAME))
            {
              OP_DAT_AccessDescriptors[OP_DATCounter] = WRITE_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_INC_NAME))
            {
              OP_DAT_AccessDescriptors[OP_DATCounter] = INC_ACCESS;
            }

            else if (iequals (variableName, OP2::OP_RW_NAME))
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

bool
ParallelLoop::isDirectLoop () const
{
  using std::map;
  using std::string;

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
  using std::string;

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

int
ParallelLoop::getNumberOfIndirectDataSets ()
{
  int numberOfIndirectDatasets = 0;

  for (unsigned int i = 1; i <= getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (OP_DAT_MappingDescriptors[i] == INDIRECT)
    {
      numberOfIndirectDatasets++;
    }
  }

  return numberOfIndirectDatasets;
}

ParallelLoop::ParallelLoop (std::string userSubroutineName,
    SgExpressionPtrList & actualArguments, Declarations * op2DeclaredVariables)
{
  this->moduleName = userSubroutineName + "_cudafor";
  this->actualArguments = actualArguments;

  retrieveOP_DATDeclarations (op2DeclaredVariables);

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
