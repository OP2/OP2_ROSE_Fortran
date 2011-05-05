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
  DIRECT, INDIRECT, GLOBAL
};

enum ACCESS_CODE_VALUE
{
  READ_ACCESS, WRITE_ACCESS, RW_ACCESS, INC_ACCESS
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
     * The same OP_DAT variable can be passed as an argument
     * to OP_PAR_LOOP. This vector records the names of the
     * unique OP_DATs
     * ======================================================
     */
    std::vector <std::string> unique_OP_DATs;

    /*
     * ======================================================
     * The base type of each OP_DAT. The map is indexed by
     * the argument group number
     * ======================================================
     */
    std::map <unsigned int, SgType *> OP_DAT_Types;

    /*
     * ======================================================
     * The dimension of each OP_DAT. The map is indexed by
     * the argument group number
     * ======================================================
     */
    std::map <unsigned int, unsigned int> OP_DAT_Dimensions;

    /*
     * ======================================================
     * The same OP_DAT variable can be passed as an argument
     * to OP_PAR_LOOP. This map records whether the OP_DAT argument
     * at this position (in the actual arguments) is a duplicate
     * of a previous OP_DAT argument
     * ======================================================
     */
    std::map <unsigned int, bool> OP_DAT_Duplicates;

    /*
     * ======================================================
     * How is the data for the OP_DAT variables in this position
     * (in the actual arguments) accessed: through
     * a mapping or directly or as a global variable?
     * ======================================================
     */
    std::map <unsigned int, MAPPING_VALUE> OP_DAT_MappingDescriptors;

    /*
     * ======================================================
     * How is the data for the OP_DAT variables in this position
     * (in the actual arguments) accessed: read, written, etc.
     * ======================================================
     */
    std::map <unsigned int, ACCESS_CODE_VALUE> OP_DAT_AccessDescriptors;

  private:

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
    getNumberOfDistinctIndirect_OP_DAT_Arguments ();

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

    SgType *
    get_OP_DAT_Type (unsigned int OP_DAT_ArgumentGroup)
    {
      return OP_DAT_Types[OP_DAT_ArgumentGroup];
    }

    unsigned int
    get_OP_DAT_Dimension (unsigned int OP_DAT_ArgumentGroup)
    {
      return OP_DAT_Dimensions[OP_DAT_ArgumentGroup];
    }

    bool
    isDuplicate_OP_DAT (unsigned int OP_DAT_ArgumentGroup)
    {
      return OP_DAT_Duplicates[OP_DAT_ArgumentGroup];
    }

    MAPPING_VALUE
    get_OP_MAP_Value (unsigned int OP_DAT_ArgumentGroup)
    {
      return OP_DAT_MappingDescriptors[OP_DAT_ArgumentGroup];
    }

    ACCESS_CODE_VALUE
    get_OP_Access_Value (unsigned int OP_DAT_ArgumentGroup)
    {
      return OP_DAT_AccessDescriptors[OP_DAT_ArgumentGroup];
    }
		
		int
		getNumberOfIndirectDataSets ( );
	
	
		
};

#endif
