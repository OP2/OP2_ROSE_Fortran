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

    /*
     * ======================================================
     * CUDA variables specific of direct loops
     * ======================================================
     */
    SgVariableDeclaration * CUDAVariable_offsetS;

    SgVariableDeclaration * CUDAVariable_warpSizeOP2;

    SgVariableDeclaration * CUDAVariable_threadSynchRet;

    /*
     * ======================================================
     * Creates the kernel call
     * ======================================================
     */
    void
    createKernelCall (KernelSubroutine & kernelSubroutine,
        ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Creates the CUDA-specific variables and initialises
     * them inside the host subroutine
     * ======================================================
     */
    void
    createCUDAVariablesDirectLoops (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Generates the declaration of the variable
     * encapsulating device variables sizes
     * ======================================================
     */
    void
    createDeviceVariablesSizesVariable (
        DataSizesDeclarationOfDirectLoop & dataSizesDeclarationOfDirectLoop);

    /*
     * ======================================================
     * Generates the code initialising the device variable
     * sizes variable
     * ======================================================
     */
    void
    initialiseDeviceVariablesSizesVariable (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Initialises: nblocks, nthreads, nshared, offsetS and
     * warpSizeOP2
     * ======================================================
     */
    void
    initialiseAllCUDAVariables (ParallelLoop & parallelLoop);

  public:

    HostSubroutineOfDirectLoop (std::string const & subroutineName,
        UserDeviceSubroutine & userDeviceSubroutine,
        KernelSubroutine & kernelSubroutine,
        DataSizesDeclarationOfDirectLoop & dataSizesDeclarationOfDirectLoop,
        ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);
};

#endif
