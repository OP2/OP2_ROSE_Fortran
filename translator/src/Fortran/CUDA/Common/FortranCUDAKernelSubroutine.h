#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_H

#include <FortranKernelSubroutine.h>
#include <ParallelLoop.h>
#include <FortranCUDADataSizesDeclaration.h>

class FortranCUDAKernelSubroutine: public FortranKernelSubroutine
{
  protected:

    void
    initialiseLocalThreadVariables ();

    void
    createReductionSubroutineCall ();

    void
    createLocalThreadDeclarations ();

    void
    createAutoSharedDeclaration ();

    FortranCUDAKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
