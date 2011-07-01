/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models a variable declaration: it includes
 * fields for the sizes of each OP_DAT dimension.
 * These size fields allow OP_DAT dimensions to be passed
 * as variables as opposed to constants
 */

#ifndef FORTRAN_OP_DAT_DIMENSIONS_DECLARATION_H
#define FORTRAN_OP_DAT_DIMENSIONS_DECLARATION_H

#include <FortranParallelLoop.h>
#include <SubroutineVariableDeclarations.h>

class FortranOpDatDimensionsDeclaration
{
  private:
    /*
     * ======================================================
     * The subroutine name
     * ======================================================
     */
    std::string subroutineName;

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
    SgDerivedTypeStatement * typeStatement;

    /*
     * ======================================================
     * Field variable declarations inside the above type
     * definition declaration statement
     * ======================================================
     */
    SubroutineVariableDeclarations * fieldDeclarations;

  private:

    void
    addFields ();

    void
    addTypeDeclaration ();

  public:

    /*
     * ======================================================
     * What is the type of the declaration?
     * ======================================================
     */
    SgClassType *
    getType ();

    /*
     * ======================================================
     * Get the field declaration representing the dimension
     * of an OP_DAT
     * ======================================================
     */
    SgVariableDeclaration *
    getOpDatDimensionField (unsigned int OP_DAT_ArgumentGroup);

    FortranOpDatDimensionsDeclaration (std::string const & subroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
