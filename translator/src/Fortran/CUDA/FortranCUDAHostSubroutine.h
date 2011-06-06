#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>
#include <FortranCUDAKernelSubroutine.h>
#include <FortranCUDAUserDeviceSubroutine.h>
#include <ParallelLoop.h>

class FortranCUDAHostSubroutine: public FortranHostSubroutine
{
  protected:

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
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
