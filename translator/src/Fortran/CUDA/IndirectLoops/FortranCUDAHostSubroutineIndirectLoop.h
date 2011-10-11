#pragma once
#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <FortranCUDAHostSubroutine.h>
#include <FortranPlan.h>

class FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop;

class FortranCUDAHostSubroutineIndirectLoop: public FortranCUDAHostSubroutine,
    public FortranPlan
{
  private:

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    void
    createPlanFunctionExecutionStatements ();

    void
    createCardinalitiesInitialisationStatements ();

    void
    createExecutionPlanDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

        FortranCUDAHostSubroutineIndirectLoop (
            SgScopeStatement * moduleScope,
            FortranKernelSubroutine * kernelSubroutine,
            FortranParallelLoop * parallelLoop,
            FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop * cardinalitiesDeclaration,
            FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
            FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
