#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>
#include <FortranCUDAKernelSubroutine.h>
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
    createThreadSynchroniseCallStatement ();

    void
    createReductionEpilogueStatements ();

    void
    createReductionPrologueStatements ();

    void
    createCUDAKernelEpilogueStatements ();

    void
    createCUDAKernelPrologueStatements ();

    void
    createCUDAKernelLocalVariableDeclarations ();

    void
    createReductionLocalVariableDeclarations ();

    void
    createDataMarshallingLocalVariableDeclarations ();

    FortranCUDAHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
