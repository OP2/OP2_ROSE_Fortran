/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models an OP_PAR_LOOP call discovered in the
 * user-supplied code
 */

#ifndef PARALLEL_LOOP_H
#define PARALLEL_LOOP_H

#include <rose.h>
#include <OP2CommonDefinitions.h>
#include <Declarations.h>
#include <OP2Variables.h>

enum MAPPING_VALUE
{
  DIRECT, INDIRECT
};

class ParallelLoop
{
  private:

    /*
     * ======================================================
     * The name of the generated Fortran/C++ module for this
     * OP_PAR_LOOP
     * ======================================================
     */
    std::string moduleName;

    /*
     * ======================================================
     * The actual arguments passed to the OP_PAR_LOOP
     * ======================================================
     */
    SgExpressionPtrList actualArguments;

    /*
     * ======================================================
     * The declarations corresponding to each distinct
     * OP_DAT argument. The key to this map is the name of the
     * actual argument passed to the OP_PAR_LOOP
     * ======================================================
     */
    std::map <std::string, OP_DAT_Declaration *> OP_DATs;

    /*
     * ======================================================
     * The same OP_DAT variable can be passed as an argument
     * to OP_PAR_LOOP. This map records how many times an OP_DAT
     * variable is passed
     * ======================================================
     */
    std::map <std::string, unsigned int> numberOfOP_DAT_Occurrences;

    /*
     * ======================================================
     * The same OP_DAT variable can be passed as an argument
     * to OP_PAR_LOOP. This map records the first OP_DAT argument
     * group where the OP_DAT is passed
     * ======================================================
     */
    std::map <std::string, unsigned int> firstOP_DAT_Occurrence;

    /*
     * ======================================================
     * How is the data in this OP_DAT variable accessed: through
     * a mapping or directly?
     * ======================================================
     */
    std::map <std::string, MAPPING_VALUE> OP_DAT_MappingDescriptors;

    /*
     * ======================================================
     * How is the data for the OP_DAT variables in this position
     * (in the actual arguments) accessed: through
     * a mapping or directly?
     * ======================================================
     */
    std::map <unsigned int, MAPPING_VALUE> OP_DAT_PositionMappingDescriptors;

  private:

    /*
     * ======================================================
     * Discovers whether all OP_MAPs are direct, and hence
     * whether this OP_PAR_LOOP has direct or indirect access
     * to its data
     * ======================================================
     */
    void
    setDirectOrIndirectLoop (Declarations * declarations);

    /*
     * ======================================================
     * Retrieves the declarations of the OP_DAT arguments
     * so that we can later retrieve their primitive types
     * and dimensions
     * ======================================================
     */
    void
    retrieveOP_DATDeclarations (Declarations * declarations);

  public:

    ParallelLoop (std::string userSubroutineName,
        SgExpressionPtrList & actualArguments,
        Declarations * op2DeclaredVariables);

    bool
    isDirectLoop () const;

    unsigned int
    getNumberOfDistinctIndirect_OP_DAT_Arguments () const;

    std::string
    getModuleName () const
    {
      return moduleName;
    }

    SgExpressionPtrList &
    getActualArguments ()
    {
      return actualArguments;
    }

    unsigned int
    getNumberOf_OP_DAT_ArgumentGroups () const
    {
      return (actualArguments.size () - OP2::NUMBER_OF_NON_OP_DAT_ARGUMENTS)
          / OP2::NUMBER_OF_ARGUMENTS_PER_OP_DAT;
    }

    unsigned int
    getNumberOfDistinct_OP_DAT_Arguments () const
    {
      return OP_DATs.size ();
    }

    std::map <std::string, OP_DAT_Declaration *>::const_iterator
    first_OP_DAT ()
    {
      return OP_DATs.begin ();
    }

    std::map <std::string, OP_DAT_Declaration *>::const_iterator
    last_OP_DAT ()
    {
      return OP_DATs.end ();
    }

    unsigned int
    getNumberOfOP_DATOccurrences (std::string const & variableName)
    {
      return numberOfOP_DAT_Occurrences[variableName];
    }

    unsigned int
    getFirstOP_DATOccurrence (std::string const & variableName)
    {
      return firstOP_DAT_Occurrence[variableName];
    }

    MAPPING_VALUE
    get_OP_MAP_Value (unsigned int positionOf_OP_DAT_InActualArguments)
    {
      return OP_DAT_PositionMappingDescriptors[positionOf_OP_DAT_InActualArguments];
    }
};

#endif
