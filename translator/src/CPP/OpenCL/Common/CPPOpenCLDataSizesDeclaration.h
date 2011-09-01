/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models a variable declaration: it includes
 * fields for the sizes of data passed to the device.
 * These sizes are needed in CPP to prevent segmentation
 * faults.
 *
 * These sizes must be passed in a struct-like data
 * structure as the number of kernel parameters can be become
 * unwieldly. In these cases, the code might not compile
 * with current-generation OpenCL compilers due to the number of
 * parameters, so the solution is to instead pack them into
 * a struct
 */

#ifndef CPP_OPENCL_DATA_SIZES_DECLARATION_H
#define CPP_OPENCL_DATA_SIZES_DECLARATION_H

#include <CPPParallelLoop.h>
#include <SubroutineVariableDeclarations.h>

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
    SubroutineVariableDeclarations * fieldDeclarations;

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
    SubroutineVariableDeclarations *
    getFieldDeclarations ();
};

#endif
