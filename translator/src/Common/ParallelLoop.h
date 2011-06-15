/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models an OP_PAR_LOOP call discovered in the
 * user-supplied code
 */

#ifndef PARALLEL_LOOP_H
#define PARALLEL_LOOP_H

#include <CommonNamespaces.h>
#include <Declarations.h>
#include <OP2Variables.h>

enum MAPPING_VALUE
{
  DIRECT, INDIRECT, GLOBAL
};

enum ACCESS_CODE_VALUE
{
  READ_ACCESS, WRITE_ACCESS, RW_ACCESS, INC_ACCESS, MAX_ACCESS, MIN_ACCESS
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
    std::vector <std::string> uniqueOpDats;

    /*
     * ======================================================
     * The base type of each OP_DAT. The map is indexed by
     * the argument group number
     * ======================================================
     */
    std::map <unsigned int, SgType *> OpDatTypes;

    /*
     * ======================================================
     * The dimension of each OP_DAT. The map is indexed by
     * the argument group number
     * ======================================================
     */
    std::map <unsigned int, unsigned int> OpDatDimensions;

    /*
     * ======================================================
     * The same OP_DAT variable can be passed as an argument
     * to OP_PAR_LOOP. This map records whether the OP_DAT argument
     * at this position (in the actual arguments) is a duplicate
     * of a previous OP_DAT argument
     * ======================================================
     */
    std::map <unsigned int, bool> OpDatDuplicates;

    /*
     * ======================================================
     * How is the data for the OP_DAT variables in this position
     * (in the actual arguments) accessed: through
     * a mapping or directly or as a global variable?
     * ======================================================
     */
    std::map <unsigned int, MAPPING_VALUE> OpDatMappingDescriptors;

    /*
     * ======================================================
     * How is the data for the OP_DAT variables in this position
     * (in the actual arguments) accessed: read, written, etc.
     * ======================================================
     */
    std::map <unsigned int, ACCESS_CODE_VALUE> OpDatAccessDescriptors;

    /*
     * ======================================================
     * What is OP_DAT variable name in this position
     * (in the actual arguments)?
     * ======================================================
     */
    std::map <unsigned int, std::string> OpDatVariableNames;

    /*
     * ======================================================
     * What is the size of the OP_DAT data types? Either it
     * is real(4) or real(8), so respectively 4 or 8
     * ======================================================
     */
    unsigned int sizeOfOpDat;

    /*
     * ======================================================
     * Mapping from an OP_DAT which requires a reduction to
     * its respective reduction subroutine
     * ======================================================
     */
    std::map <unsigned int, SgProcedureHeaderStatement *> reductionSubroutines;

  private:

    /*
     * ======================================================
     * Retrieves the declarations of the OP_DAT arguments
     * so that we can later retrieve their primitive types
     * and dimensions
     * ======================================================
     */
    void
    retrieveOpDatDeclarations (Declarations * declarations);

  public:

    /*
     * ======================================================
     * Generates all reduction subroutines: one per OP_DAT
     * type and kind
     * ======================================================
     */
    void
    generateReductionSubroutines (SgScopeStatement * moduleScope);

    bool
    isDirectLoop () const;

    unsigned int
    getNumberOfDistinctIndirectOpDatArguments ();

    std::string
    getModuleName () const;

    SgExpressionPtrList &
    getActualArguments ();

    unsigned int
    getNumberOfOpDatArgumentGroups () const;

    SgType *
    getOpDatType (unsigned int OP_DAT_ArgumentGroup);

    unsigned int
    getOpDatDimension (unsigned int OP_DAT_ArgumentGroup);

    bool
    isDuplicateOpDat (unsigned int OP_DAT_ArgumentGroup);

    MAPPING_VALUE
    getOpMapValue (unsigned int OP_DAT_ArgumentGroup);

    ACCESS_CODE_VALUE
    getOpAccessValue (unsigned int OP_DAT_ArgumentGroup);

    unsigned int
    getNumberOfIndirectDataSets ();

    unsigned int
    getNumberOfDifferentIndirectDataSets ();

    bool
    isReductionRequired ();

    bool
    isReductionRequired (int OP_DAT_ArgumentGroup);

    std::string
    getOpDatVariableName (unsigned int OP_DAT_ArgumentGroup);

    unsigned int
    getSizeOfOpDat () const;

    SgProcedureHeaderStatement *
    getReductionSubroutineHeader (unsigned int OP_DAT_ArgumentGroup);

    ParallelLoop (std::string userSubroutineName,
        SgExpressionPtrList & actualArguments, Declarations * declarations);
};

#endif
