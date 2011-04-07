#include <boost/algorithm/string.hpp>
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
ParallelLoop::retrieveOP_DATDeclarations (Declarations * op2DeclaredVariables)
{
  using boost::iequals;
  using std::string;
  using std::vector;
  using std::map;
  using std::make_pair;

  Debug::getInstance ()->debugMessage ("Retrieving OP_DAT declarations", 2);

  int OP_DATCounter = -1;

  string current_OP_DAT_VariableName;

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

        if (variableReference->get_type ()->variantT () == V_SgClassType)
        {
          SgClassType * classReference = isSgClassType (
              variableReference->get_type ());

          string const variableName =
              variableReference->get_symbol ()->get_name ().getString ();

          string const className = classReference->get_name ().getString ();

          if (iequals (className, OP2::OP_DAT_NAME))
          {
            current_OP_DAT_VariableName = variableName;

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
                  op2DeclaredVariables->get_OP_DAT_Declaration (variableName);

              if (OP_DATs.count (variableName) == 0)
              {
                OP_DATs.insert (make_pair (variableName, opDatDeclaration));

                numberOfOP_DAT_Occurrences.insert (make_pair (variableName, 1));

                firstOP_DAT_Occurrence.insert (make_pair (variableName,
                    OP_DATCounter));
              }
              else
              {
                unsigned int occurrences =
                    numberOfOP_DAT_Occurrences[variableName] + 1;

                numberOfOP_DAT_Occurrences[variableName] = occurrences;
              }
            }
            catch (const std::string & exception)
            {
              Debug::getInstance ()->debugMessage ("'" + variableName
                  + "' must have been declared through OP_DECL_GBL", 1);
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
              OP_DAT_MappingDescriptors.insert (make_pair (
                  current_OP_DAT_VariableName, DIRECT));

              OP_DAT_PositionMappingDescriptors[OP_DATCounter] = DIRECT;
            }
            else
            {
              OP_DAT_MappingDescriptors.insert (make_pair (
                  current_OP_DAT_VariableName, INDIRECT));

              OP_DAT_PositionMappingDescriptors[OP_DATCounter] = INDIRECT;
            }
          }
        }
      }

      default:
      {

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

  for (map <string, MAPPING_VALUE>::const_iterator it =
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
ParallelLoop::getNumberOfDistinctIndirect_OP_DAT_Arguments () const
{
  using std::map;
  using std::string;

  unsigned int count = 0;

  for (map <string, MAPPING_VALUE>::const_iterator it =
      OP_DAT_MappingDescriptors.begin (); it
      != OP_DAT_MappingDescriptors.end (); ++it)
  {
    if (it->second == INDIRECT)
    {
      count++;
    }
  }

  return count;
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
