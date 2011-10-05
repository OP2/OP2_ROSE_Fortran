#pragma once
#ifndef CPP_OP_DAT_DIMENSIONS_DECLARATION_H
#define CPP_OP_DAT_DIMENSIONS_DECLARATION_H

#include <CPPParallelLoop.h>
#include <SubroutineVariableDeclarations.h>

class CPPOpDatDimensionsDeclaration
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
    CPPParallelLoop * parallelLoop;

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

    CPPOpDatDimensionsDeclaration (std::string const & subroutineName,
        CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
