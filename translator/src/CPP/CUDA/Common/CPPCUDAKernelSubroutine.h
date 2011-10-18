#pragma once
#ifndef CPP_CUDA_KERNEL_SUBROUTINE_H
#define CPP_CUDA_KERNEL_SUBROUTINE_H

#include <CPPKernelSubroutine.h>

class CPPCUDAUserSubroutine;
class CPPReductionSubroutines;

class CPPCUDAKernelSubroutine: public CPPKernelSubroutine
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

    CPPCUDAKernelSubroutine (SgScopeStatement * moduleScope,
        CPPCUDAUserSubroutine * userSubroutine, CPPParallelLoop * parallelLoop);

};

#endif
