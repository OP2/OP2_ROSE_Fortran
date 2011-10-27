#pragma once
#ifndef FORTRAN_OP_DAT_DIMENSIONS_DECLARATION_H
#define FORTRAN_OP_DAT_DIMENSIONS_DECLARATION_H

class SgClassType;
class SgScopeStatement;
class SgDerivedTypeStatement;
class SgVarRefExp;
class FortranParallelLoop;
class ScopedVariableDeclarations;

class FortranOpDatDimensionsDeclaration
{
  private:

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
    ScopedVariableDeclarations * fieldDeclarations;

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
     * Get a reference to the field representing the dimension
     * of an OP_DAT
     * ======================================================
     */
    SgVarRefExp *
    getOpDatDimensionField (unsigned int OP_DAT_ArgumentGroup);

    FortranOpDatDimensionsDeclaration (FortranParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
