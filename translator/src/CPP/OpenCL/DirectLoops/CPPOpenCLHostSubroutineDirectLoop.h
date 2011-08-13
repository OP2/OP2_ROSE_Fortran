/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the host subroutine for a direct loop.
 * Its declarations and statements do the following:
 *
 * 1) Set up OpenCL run-time variables, such as block and grid
 * sizes
 *
 * 2) Allocates device memory for the user subroutine formal
 * parameters
 *
 * 3) Calls the kernel
 *
 * 4) Deallocates device memory
 */

#ifndef CPP_OpenCL_HOST_SUBROUTINE_DIRECT_LOOP_H
#define CPP_OpenCL_HOST_SUBROUTINE_DIRECT_LOOP_H



#include <CPPOpenCLHostSubroutine.h>
#include <CPPOpenCLDataSizesDeclarationDirectLoop.h>
#include <CPPInitialiseConstantsSubroutine.h>
#include <CPPOpenCLModuleDeclarations.h>
#include <CPPOpDatDimensionsDeclaration.h>

class CPPOpenCLHostSubroutineDirectLoop: public CPPOpenCLHostSubroutine
{
  private:

    /*
     * ======================================================
     * Generates the code initialising the device variable
     * sizes variable
     * ======================================================
     */
    void
    createVariableSizesInitialisationStatements ();

    /*
     * ======================================================
     * Initialises OpenCL-specific variables
     * ======================================================
     */
    void
    createOpenCLKernelInitialisationStatements ();

    /*
     * ======================================================
     * Creates the additional OpenCL-specific variables and
     * initialises them inside the host subroutine
     * ======================================================
     */
    void
    createOpenCLKernelLocalVariableDeclarationsForDirectLoop ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

        CPPOpenCLHostSubroutineDirectLoop (
            std::string const & subroutineName,
            std::string const & userSubroutineName,
            std::string const & kernelSubroutineName,
            CPPParallelLoop * parallelLoop,
            SgScopeStatement * moduleScope,
            CPPInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
            CPPOpenCLDataSizesDeclarationDirectLoop * dataSizesDeclaration,
            CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration);
            /*CPPOpenCLModuleDeclarations * moduleDeclarations);*/
};

#endif
