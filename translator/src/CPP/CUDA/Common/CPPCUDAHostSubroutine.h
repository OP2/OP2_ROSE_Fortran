#pragma once
#ifndef CPP_CUDA_HOST_SUBROUTINE_H
#define CPP_CUDA_HOST_SUBROUTINE_H

#include <CPPHostSubroutine.h>

class CPPCUDAKernelSubroutine;
class CPPModuleDeclarations;

class CPPCUDAHostSubroutine: public CPPHostSubroutine
{
  protected:

    CPPModuleDeclarations * moduleDeclarations;

  protected:

    SgForStatement *
    createReductionUpdateStatements (unsigned int OP_DAT_ArgumentGroup);

    virtual void
    createReductionEpilogueStatements ();

    SgForStatement *
    createReductionInitialisationStatements (unsigned int OP_DAT_ArgumentGroup);

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionDeclarations ();

    void
    createCUDAConfigurationLaunchDeclarations ();

    CPPCUDAHostSubroutine (SgScopeStatement * moduleScope,
        CPPCUDAKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPModuleDeclarations * moduleDeclarations);
};

#endif
