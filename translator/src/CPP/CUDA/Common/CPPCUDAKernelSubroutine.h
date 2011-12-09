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
     * Creates the variable declarations needed to perform
     * reductions
     * ======================================================
     */
    void
    createReductionVariableDeclarations ();

    CPPCUDAKernelSubroutine (SgScopeStatement * moduleScope,
        CPPCUDAUserSubroutine * userSubroutine, CPPParallelLoop * parallelLoop,
        CPPReductionSubroutines * reductionSubroutines);

};

#endif
