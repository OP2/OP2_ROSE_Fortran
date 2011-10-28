#pragma once
#ifndef CPP_CUDA_KERNEL_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_CUDA_KERNEL_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPCUDAKernelSubroutine.h>

class CPPCUDAKernelSubroutineIndirectLoop: public CPPCUDAKernelSubroutine
{
  private:

    std::map <std::string, SgVariableDeclaration *>
        indirectOpDatSharedMemoryDeclarations;

  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    SgBasicBlock *
    createIncrementAndWriteAccessEpilogueStatements ();

    SgBasicBlock *
    createStageOutFromLocalMemoryToSharedMemoryStatements ();

    SgBasicBlock *
    createInitialiseIncrementAccessStatements ();

    virtual void
    createExecutionLoopStatements ();

    SgBasicBlock *
    createInitialiseCUDASharedVariablesStatements ();

    SgBasicBlock *
    createSetIndirectionMapPointerStatements ();

    SgBasicBlock *
    createSetOpDatSharedMemoryPointerStatements ();

    SgBasicBlock *
    createSetNumberOfIndirectElementsPerBlockStatements ();

    SgBasicBlock *
    createIncrementAccessThreadZeroStatements ();

    SgIfStmt *
    createThreadZeroStatements ();

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

    CPPCUDAKernelSubroutineIndirectLoop (SgScopeStatement * moduleScope,
        CPPCUDAUserSubroutine * userSubroutine, CPPParallelLoop * parallelLoop,
        CPPCUDAReductionSubroutines * reductionSubroutines);
};

#endif
