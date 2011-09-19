#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_H

#include <FortranKernelSubroutine.h>
#include <FortranParallelLoop.h>
#include <FortranCUDADataSizesDeclaration.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranCUDAModuleDeclarations.h>

class FortranCUDAKernelSubroutine: public FortranKernelSubroutine
{
  protected:

    FortranCUDADataSizesDeclaration * dataSizesDeclaration;

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
        FortranCUDADataSizesDeclaration * dataSizesDeclaration,
        FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
