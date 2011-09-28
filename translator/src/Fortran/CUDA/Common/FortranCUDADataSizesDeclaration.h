/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models a type declaration in Fortran.
 *
 * The type declaration includes a field to store the
 * cardinality of each OP_DAT. The cardinality of an OP_DAT
 * equals <OP_SET size> * <dimension of OP_DAT>.
 *
 * These cardinalities are needed in Fortran to prevent
 * segmentation faults
 */

#ifndef FORTRAN_CUDA_DATA_SIZES_DECLARATION_H
#define FORTRAN_CUDA_DATA_SIZES_DECLARATION_H

#include <FortranParallelLoop.h>
#include <SubroutineVariableDeclarations.h>

class FortranCUDAOpDatCardinalitiesDeclaration
{
  protected:

    /*
     * ======================================================
     * The parallel loop
     * ======================================================
     */
    FortranParallelLoop * parallelLoop;

    /*
     * ======================================================
     * The scope of the declaration
     * ======================================================
     */
    SgScopeStatement * moduleScope;

    /*
     * ======================================================
     * The type definition declaration statement
     * ======================================================
     */
    SgDerivedTypeStatement * deviceDatatypeStatement;

    /*
     * ======================================================
     * Field variable declarations inside the above type
     * definition declaration statement
     * ======================================================
     */
    SubroutineVariableDeclarations * fieldDeclarations;

  protected:

    void
    addFields ();

  public:

    /*
     * ======================================================
     * What is the type of the data sizes declaration?
     * ======================================================
     */
    SgClassType *
    getType ();

    /*
     * ======================================================
     * Get the field declarations
     * ======================================================
     */
    SubroutineVariableDeclarations *
    getFieldDeclarations ();

    FortranCUDAOpDatCardinalitiesDeclaration (
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
