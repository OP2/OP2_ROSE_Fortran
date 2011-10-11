#pragma once
#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <FortranCUDAHostSubroutine.h>

class FortranCUDAModuleDeclarations;

class FortranCUDAHostSubroutineDirectLoop: public FortranCUDAHostSubroutine
{
  private:

    /*
     * ======================================================
     * Initialises CUDA-specific variables
     * ======================================================
     */
    void
    createCUDAKernelInitialisationStatements ();

    /*
     * ======================================================
     * Creates the additional CUDA-specific variables and
     * initialises them inside the host subroutine
     * ======================================================
     */
    void
    createCUDAKernelLocalVariableDeclarationsForDirectLoop ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

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
