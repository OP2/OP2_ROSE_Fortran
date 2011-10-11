#pragma once
#ifndef CPP_CUDA_HOST_SUBROUTINE_H
#define CPP_CUDA_HOST_SUBROUTINE_H

#include <CPPHostSubroutine.h>

class CPPCUDAKernelSubroutine;

class CPPCUDAHostSubroutine: public CPPHostSubroutine
{
  protected:

    virtual void
    createReductionEpilogueStatements ();

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionLocalVariableDeclarations ();

    void
    createCUDAKernelLocalVariableDeclarations ();

    CPPCUDAHostSubroutine (SgScopeStatement * moduleScope,
        CPPCUDAKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop);
};

#endif
