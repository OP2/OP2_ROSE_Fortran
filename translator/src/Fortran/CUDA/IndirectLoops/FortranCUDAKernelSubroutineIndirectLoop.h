#pragma once
#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_INDIRECT_LOOP_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_INDIRECT_LOOP_H

#include <FortranCUDAKernelSubroutine.h>

class FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop;

class FortranCUDAKernelSubroutineIndirectLoop: public FortranCUDAKernelSubroutine
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

        FortranCUDAKernelSubroutineIndirectLoop (
            SgScopeStatement * moduleScope,
            FortranCUDAUserSubroutine * userSubroutine,
            FortranParallelLoop * parallelLoop,
            FortranReductionSubroutines * reductionSubroutines,
            FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop * cardinalitiesDeclaration,
            FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
            FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
