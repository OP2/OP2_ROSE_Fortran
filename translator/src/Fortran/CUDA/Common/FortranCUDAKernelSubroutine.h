#pragma once
#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_H

#include <FortranKernelSubroutine.h>

class FortranCUDAOpDatCardinalitiesDeclaration;
class FortranCUDAModuleDeclarations;
class FortranOpDatDimensionsDeclaration;
class FortranReductionSubroutines;

class FortranCUDAKernelSubroutine: public FortranKernelSubroutine
{
  protected:

    FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration;

    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration;

    FortranCUDAModuleDeclarations * moduleDeclarations;

  protected:

    /*
     * ======================================================
     * For this OP_GBL, create the expression
     * ======================================================
     */
    SgExpression *
    createUserKernelOpGlobalActualParameterExpression (
        unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Creates the statements which initialises the variables
     * used to stage in data from device->shared->stack memory
     * ======================================================
     */
    void
    createInitialiseCUDAStageInVariablesStatements ();

    /*
     * ======================================================
     * Creates the statements executed after the call to the
     * user subroutine to perform the thread-block reduction
     * ======================================================
     */
    void
    createReductionEpilogueStatements ();

    /*
     * ======================================================
     * Creates the variable declarations needed to stage in
     * data from device->shared->stack memory
     * ======================================================
     */
    void
    createCUDAStageInVariablesVariableDeclarations ();

    /*
     * ======================================================
     * Creates the variable declarations needed in CUDA
     * shared memory
     * ======================================================
     */
    void
    createCUDASharedVariableDeclarations ();

    FortranCUDAKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranReductionSubroutines * reductionSubroutines,
        FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
        FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
