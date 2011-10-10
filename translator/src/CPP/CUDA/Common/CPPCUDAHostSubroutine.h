#pragma once
#ifndef CPP_CUDA_HOST_SUBROUTINE_H
#define CPP_CUDA_HOST_SUBROUTINE_H

#include <CPPHostSubroutine.h>

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

    CPPCUDAHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
