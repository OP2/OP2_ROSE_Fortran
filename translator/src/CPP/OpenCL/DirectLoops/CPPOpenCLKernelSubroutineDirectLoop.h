#pragma once
#ifndef CPP_OPENCL_KERNEL_SUBROUTINE_DIRECT_LOOP_H
#define CPP_OPENCL_KERNEL_SUBROUTINE_DIRECT_LOOP_H

#include <CPPOpenCLKernelSubroutine.h>

class CPPOpenCLKernelSubroutineDirectLoop: public CPPOpenCLKernelSubroutine
{
  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    SgForStatement *
    createStageInFromDeviceMemoryToSharedMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    SgForStatement *
    createStageInFromSharedMemoryToLocalMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    SgForStatement *
    createStageOutFromSharedMemoryToDeviceMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    SgForStatement *
    createStageOutFromLocalMemoryToSharedMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    virtual void
    createExecutionLoopStatements ();

    void
    createInitialiseOffsetIntoOpenCLSharedVariableStatements ();

    void
    createThreadIDInitialisationStatement ();

    virtual void
    createStatements ();

    void
    createOpenCLSharedVariableDeclarations ();

    void
    createStageInVariableDeclarations ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createOpDatFormalParameterDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPOpenCLKernelSubroutineDirectLoop (SgScopeStatement * moduleScope,
        CPPOpenCLUserSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPReductionSubroutines * reductionSubroutines);
};

#endif
