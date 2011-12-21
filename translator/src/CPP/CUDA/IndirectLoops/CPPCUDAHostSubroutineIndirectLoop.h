#pragma once
#ifndef CPP_CUDA_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_CUDA_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPCUDAHostSubroutine.h>

class CPPCUDAKernelSubroutine;

class CPPCUDAHostSubroutineIndirectLoop: public CPPCUDAHostSubroutine
{

  private:

    virtual void
    createKernelFunctionCallStatement (SgScopeStatement * scope);

    SgBasicBlock *
    createPlanFunctionExecutionStatements ();

    SgStatement *
    createPlanFunctionCallStatement ();

    virtual void
    createStatements ();

    void
    createPlanFunctionDeclarations ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPCUDAHostSubroutineIndirectLoop (SgScopeStatement * moduleScope,
        CPPCUDAKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPModuleDeclarations * moduleDeclarations);
};

#endif
