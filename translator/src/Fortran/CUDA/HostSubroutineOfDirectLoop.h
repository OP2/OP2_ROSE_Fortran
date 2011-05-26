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

#ifndef HOST_SUBROUTINE_OF_DIRECT_LOOP_H
#define HOST_SUBROUTINE_OF_DIRECT_LOOP_H

#include <HostSubroutine.h>
#include <KernelSubroutine.h>
#include <DataSizesDeclarationOfDirectLoop.h>

class HostSubroutineOfDirectLoop: public HostSubroutine
{
  private:

    DataSizesDeclarationOfDirectLoop * dataSizesDeclarationOfDirectLoop;

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
     * Creates the CUDA-specific variables and initialises
     * them inside the host subroutine
     * ======================================================
     */
    void
    createCUDAVariablesDirectLoops ();

    /*
     * ======================================================
     * Generates the declaration of the variable
     * encapsulating device variables sizes
     * ======================================================
     */
    void
    createDeviceVariablesSizesVariable ();

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

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    HostSubroutineOfDirectLoop (std::string const & subroutineName,
        UserDeviceSubroutine * userDeviceSubroutine,
        KernelSubroutine * kernelSubroutine,
        DataSizesDeclarationOfDirectLoop * dataSizesDeclarationOfDirectLoop,
        ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
