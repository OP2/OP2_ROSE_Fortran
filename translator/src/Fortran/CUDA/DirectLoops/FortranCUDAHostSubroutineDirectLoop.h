/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the host subroutine for a direct loop.
 * Its declarations and statements do the following:
 *
 * 1) Set up CUDA run-time variables, such as block and grid
 * sizes
 *
 * 2) Allocates device memory for the user subroutine formal
 * parameters
 *
 * 3) Calls the kernel
 *
 * 4) Deallocates device memory
 */

#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <FortranCUDAHostSubroutine.h>
#include <FortranCUDAKernelSubroutine.h>
#include <FortranCUDADataSizesDeclarationDirectLoop.h>

class FortranCUDAHostSubroutineDirectLoop: public FortranCUDAHostSubroutine
{
  private:

    FortranCUDADataSizesDeclarationDirectLoop
        * dataSizesDeclarationOfDirectLoop;

  private:

    /*
     * ======================================================
     * Creates the kernel call
     * ======================================================
     */
    void
    createKernelCall ();

    /*
     * ======================================================
     * Generates the code initialising the device variable
     * sizes variable
     * ======================================================
     */
    void
    initialiseDeviceVariablesSizesVariable ();

    /*
     * ======================================================
     * Initialises CUDA-specific variables
     * ======================================================
     */
    void
    initialiseAllCUDAVariables ();

    /*
     * ======================================================
     * Creates the additional CUDA-specific variables and
     * initialises them inside the host subroutine
     * ======================================================
     */
    void
    createAdditionalCUDAKernelVariableDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

        FortranCUDAHostSubroutineDirectLoop (
            std::string const & subroutineName,
            std::string const & userSubroutineName,
            std::string const & kernelSubroutineName,
            FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclarationOfDirectLoop,
            ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
