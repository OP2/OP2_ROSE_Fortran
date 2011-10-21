#pragma once
#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_INDIRECT_LOOP_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_INDIRECT_LOOP_H

#include <FortranCUDAKernelSubroutine.h>

class FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop;

class FortranCUDAKernelSubroutineIndirectLoop: public FortranCUDAKernelSubroutine
{
  private:

    std::map <std::string, unsigned int> positionOfNbytes;

  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    SgBasicBlock *
    createIncrementAndWriteAccessEpilogueStatements ();

    SgBasicBlock *
    createStageOutFromLocalMemoryToSharedMemoryStatements ();

    SgBasicBlock *
    createIncrementAdjustmentStatements ();

    SgBasicBlock *
    createInitialiseIncrementAccessStatements ();

    virtual void
    createExecutionLoopStatements ();

    SgBasicBlock *
    createInitialiseCUDASharedVariablesStatements ();

    SgBasicBlock *
    createIncrementAccessThreadZeroStatements ();

    SgIfStmt *
    createThreadZeroStatements ();

    SgBasicBlock *
    createInitialiseBytesPerOpDatStatements ();

    void
    createIncrementAccessLocalVariableDeclarations ();

    void
    createExecutionLocalVariableDeclarations ();

    void
    createPlanFormalParameterDeclarations ();

    virtual void
    createOpDatFormalParameterDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

        FortranCUDAKernelSubroutineIndirectLoop (
            SgScopeStatement * moduleScope,
            FortranCUDAUserSubroutine * userSubroutine,
            FortranParallelLoop * parallelLoop,
            FortranReductionSubroutines * reductionSubroutines,
            FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop * cardinalitiesDeclaration,
            FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
            FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
