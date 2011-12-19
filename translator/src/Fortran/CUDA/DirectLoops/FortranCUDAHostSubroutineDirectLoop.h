#pragma once
#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <FortranCUDAHostSubroutine.h>

class FortranCUDAModuleDeclarations;

class FortranCUDAHostSubroutineDirectLoop: public FortranCUDAHostSubroutine
{
  private:

    void
    createCUDAKernelInitialisationStatements ();

    void
    createCUDAKernelActualParameterDeclarations ();

    virtual void
    createKernelFunctionCallStatement (SgScopeStatement * scope);

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    FortranCUDAHostSubroutineDirectLoop (SgScopeStatement * moduleScope,
        FortranKernelSubroutine * kernelSubroutine,
        FortranParallelLoop * parallelLoop,
        FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration,
        FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
