#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>
#include <FortranCUDAKernelSubroutine.h>
#include <FortranCUDAUserDeviceSubroutine.h>
#include <ParallelLoop.h>

class FortranCUDAHostSubroutine: public FortranHostSubroutine
{
  protected:

    FortranCUDAKernelSubroutine * kernelSubroutine;

  protected:

    /*
     * ======================================================
     * Returns a statement which represents a C-to-Fortran
     * pointer conversion
     * ======================================================
     */
    SgStatement *
    createCToFortranPointerCall (SgExpression * parameter1,
        SgExpression * parameter2, SgExpression * parameter3 = NULL);

    /*
     * ======================================================
     * Returns a statement which represents a CUDA thread
     * synchronisation call
     * ======================================================
     */
    SgStatement *
    createThreadSynchroniseCall ();

    void
    copyDataBackFromDeviceAndDeallocate ();

    void
    createSupportForReductionVariablesBeforeKernel ();

    void
    createSupportForReductionVariablesAfterKernel ();

    void
    initialiseDataMarshallingVariables ();

    void
    createReductionIterationVariableDeclarations ();

    void
    createDataMarshallingDeclarations ();

    void
    createCUDAKernelVariableDeclarations ();

    void
    createReductionVariableDeclarations ();

    FortranCUDAHostSubroutine (std::string const & subroutineName,
        FortranCUDAUserDeviceSubroutine * userDeviceSubroutine,
        FortranCUDAKernelSubroutine * kernelSubroutine,
        ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
