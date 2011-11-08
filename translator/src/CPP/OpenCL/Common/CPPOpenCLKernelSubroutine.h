#pragma once
#ifndef CPP_OPENCL_KERNEL_SUBROUTINE_H
#define CPP_OPENCL_KERNEL_SUBROUTINE_H

#include <CPPKernelSubroutine.h>

class CPPOpenCLUserSubroutine;
class CPPReductionSubroutines;

class CPPOpenCLKernelSubroutine: public CPPKernelSubroutine
{
  protected:

    /*
     * ======================================================
     * Creates the statements which initialises the variables
     * used to stage in data from device->shared->stack memory
     * ======================================================
     */
    void
    createInitialiseCUDAStageInVariablesStatements ();

    /*
     * ======================================================
     * Creates the statements executed before the call to the
     * user subroutine to initialise the local thread variables
     * to zero
     * ======================================================
     */
    void
    createReductionPrologueStatements ();

    /*
     * ======================================================
     * Creates the statements executed after the call to the
     * user subroutine to perform the thread-block reduction
     * ======================================================
     */
    void
    createReductionEpilogueStatements ();

    /*
     * ======================================================
     * Creates the variable declarations needed to stage in
     * data from device->shared->stack memory
     * ======================================================
     */
    void
    createCUDAStageInVariablesVariableDeclarations ();

    /*
     * ======================================================
     * Creates the variable declarations needed in CUDA
     * shared memory
     * ======================================================
     */
    void
    createCUDASharedVariableDeclarations ();

    CPPOpenCLKernelSubroutine (SgScopeStatement * moduleScope,
        CPPOpenCLUserSubroutine * userSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPReductionSubroutines * reductionSubroutines);
};

#endif
