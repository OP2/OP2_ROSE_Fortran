#pragma once
#ifndef CPP_CUDA_HOST_SUBROUTINE_DIRECT_LOOP_H
#define CPP_CUDA_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <CPPCUDAHostSubroutine.h>

class CPPCUDAKernelSubroutine;

class CPPCUDAHostSubroutineDirectLoop: public CPPCUDAHostSubroutine
{
  private:

    void
    createCUDAKernelInitialisationStatements ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPCUDAHostSubroutineDirectLoop (SgScopeStatement * moduleScope,
        CPPCUDAKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop);
};

#endif
