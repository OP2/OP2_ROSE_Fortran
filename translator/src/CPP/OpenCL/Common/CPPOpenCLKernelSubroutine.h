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

    CPPOpenCLKernelSubroutine (SgScopeStatement * moduleScope,
        CPPOpenCLUserSubroutine * userSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPReductionSubroutines * reductionSubroutines);
};

#endif
