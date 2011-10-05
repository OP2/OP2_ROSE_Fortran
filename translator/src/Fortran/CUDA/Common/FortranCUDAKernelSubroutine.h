
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

    SgExpression *
    buildOpGlobalActualParameterExpression (unsigned int OP_DAT_ArgumentGroup);

    void
    createInitialiseLocalThreadVariablesStatements ();

    void
    createReductionEpilogueStatements ();

    void
    createLocalThreadDeclarations ();

    void
    createAutoSharedDeclarations ();

    FortranCUDAKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranReductionSubroutines * reductionSubroutines,
        FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
        FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
