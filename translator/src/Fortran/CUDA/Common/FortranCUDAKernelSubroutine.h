#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_H

#include <FortranKernelSubroutine.h>
#include <ParallelLoop.h>
#include <FortranCUDADataSizesDeclaration.h>
#include <FortranOpDatDimensionsDeclaration.h>

class FortranCUDAKernelSubroutine: public FortranKernelSubroutine
{
  protected:

    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration;

  protected:

    void
    createInitialiseLocalThreadVariablesStatements ();

    void
    createReductionLoopStatements ();

    void
    createLocalThreadDeclarations ();

    void
    createAutoSharedDeclaration ();

    FortranCUDAKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope,
        FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration);
};

#endif
