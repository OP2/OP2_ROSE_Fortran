#pragma once
#ifndef CPP_OPENCL_DATA_SIZES_DECLARATION_H
#define CPP_OPENCL_DATA_SIZES_DECLARATION_H

#include <string>

class SgScopeStatement;
class SgDerivedTypeStatement;
class SgClassType;
class CPPParallelLoop;
class ScopedVariableDeclarations;

class CPPOpenCLDataSizesDeclaration
{
  protected:

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
    SgDerivedTypeStatement * deviceDatatypeStatement;

    /*
     * ======================================================
     * Field variable declarations inside the above type
     * definition declaration statement
     * ======================================================
     */
    ScopedVariableDeclarations * fieldDeclarations;

  protected:

    virtual void
    addFields () = 0;

    CPPOpenCLDataSizesDeclaration (std::string const & subroutineName,
        CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope);

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
    ScopedVariableDeclarations *
    getFieldDeclarations ();
};

#endif
