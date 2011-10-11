#pragma once
#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_DIRECT_LOOP_H

#include <FortranCUDAKernelSubroutine.h>

class FortranCUDAKernelSubroutineDirectLoop: public FortranCUDAKernelSubroutine
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
    SgFortranDo *
    createStageInFromDeviceMemoryToSharedMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Builds the statements which stages in data from shared
     * memory into local memory for the OP_DAT in this
     * argument group
     * ======================================================
     */
    SgFortranDo *
    createStageInFromSharedMemoryToLocalMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Builds the statements which stages out data from shared
     * memory into device memory for the OP_DAT in this
     * argument group
     * ======================================================
     */
    SgFortranDo *
    createStageOutFromSharedMemoryToDeviceMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Builds the statements which stages out data from local
     * memory into shared memory for the OP_DAT in this
     * argument group
     * ======================================================
     */
    SgFortranDo *
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

    FortranCUDAKernelSubroutineDirectLoop (SgScopeStatement * moduleScope,
        FortranCUDAUserSubroutine * userSubroutine,
        FortranParallelLoop * parallelLoop,
        FortranReductionSubroutines * reductionSubroutines,
        FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration,
        FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
