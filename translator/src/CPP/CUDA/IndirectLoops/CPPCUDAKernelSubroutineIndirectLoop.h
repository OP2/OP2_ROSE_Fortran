#pragma once
#ifndef CPP_CUDA_KERNEL_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_CUDA_KERNEL_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPCUDAKernelSubroutine.h>

class CPPCUDAKernelSubroutineIndirectLoop: public CPPCUDAKernelSubroutine
{
  private:

    std::map <std::string, unsigned int> positionOfNbytes;

  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    virtual void
    createExecutionLoopStatements ();

    void
    createInnerExecutionLoopStatements (SgScopeStatement * scope);

    void
    createIncrementAdjustmentStatements (SgScopeStatement * scope);

    void
    createInitialiseLocalOpDatStatements (SgScopeStatement * scope);

    void
    createPointeredIncrementsOrWritesStatements ();

    void
    createInitialiseCUDASharedVariablesStatements ();

    void
    createInitialiseLocalVariablesStatements ();

    void
    createThreadZeroStatements ();

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
        CPPCUDAUserSubroutine * userSubroutine, CPPParallelLoop * parallelLoop);
};

#endif
