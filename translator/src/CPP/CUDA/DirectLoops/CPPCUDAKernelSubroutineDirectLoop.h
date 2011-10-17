#pragma once
#ifndef CPP_CUDA_KERNEL_SUBROUTINE_DIRECT_LOOP_H
#define CPP_CUDA_KERNEL_SUBROUTINE_DIRECT_LOOP_H

#include <CPPCUDAKernelSubroutine.h>

class CPPCUDAKernelSubroutineDirectLoop: public CPPCUDAKernelSubroutine
{
  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    virtual void
    createExecutionLoopStatements ();

    /*
     * ======================================================
     * Builds the statements which stages in data from device
     * memory into shared memory for the OP_DAT in this
     * argument group
     * ======================================================
     */

    SgForStatement *
    createStageInFromDeviceMemoryToSharedMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Builds the statements which stages in data from shared
     * memory into local memory for the OP_DAT in this
     * argument group
     * ======================================================
     */

    SgForStatement *
    createStageInFromSharedMemoryToLocalMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Builds the statements which stages out data from shared
     * memory into device memory for the OP_DAT in this
     * argument group
     * ======================================================
     */

    SgForStatement *
    createStageOutFromSharedMemoryToDeviceMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Builds the statements which stages out data from local
     * memory into shared memory for the OP_DAT in this
     * argument group
     * ======================================================
     */

    SgForStatement *
    createStageOutFromLocalMemoryToSharedMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    void
    createInitialiseOffsetIntoCUDASharedVariableStatements ();

    void
    createThreadIDInitialisationStatement ();

    virtual void
    createOpDatFormalParameterDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPCUDAKernelSubroutineDirectLoop (SgScopeStatement * moduleScope,
        CPPCUDAUserSubroutine * userSubroutine, CPPParallelLoop * parallelLoop,
        CPPReductionSubroutines * reductionSubroutines);
};

#endif
