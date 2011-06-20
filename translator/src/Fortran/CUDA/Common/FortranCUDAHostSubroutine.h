#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>
#include <FortranCUDAKernelSubroutine.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <ParallelLoop.h>

class FortranCUDAHostSubroutine: public FortranHostSubroutine
{
  protected:

    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration;

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
    createOpDatDimensionInitialisationStatements ();

    void
    createCUDAKernelLocalVariableDeclarations ();

    void
    createReductionLocalVariableDeclarations ();

    void
    createDataMarshallingLocalVariableDeclarations ();

    FortranCUDAHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope,     FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration);
};

#endif
