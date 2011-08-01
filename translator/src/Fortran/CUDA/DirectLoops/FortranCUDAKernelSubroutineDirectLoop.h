/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the kernel subroutine for a direct loop
 */

#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_DIRECT_LOOP_H

#include <FortranCUDAKernelSubroutine.h>
#include <FortranCUDADataSizesDeclarationDirectLoop.h>

class FortranCUDAKernelSubroutineDirectLoop: public FortranCUDAKernelSubroutine
{
  private:

    FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclaration;

  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    virtual void
    createExecutionLoopStatements ();

    /*
     * ======================================================
     * Builds the assignments of shared memory and local
     * thread variables
     * ======================================================
     */
    SgBasicBlock *
    createStageInFromDeviceMemoryToLocalThreadVariablesStatements ();

    /*
     * ======================================================
     * Builds the assignments of shared memory and local
     * thread variables
     * ======================================================
     */
    SgBasicBlock *
    createStageOutFromLocalThreadVariablesToDeviceMemoryStatements ();

    void
    createAutoSharedDisplacementInitialisationStatement ();

    void
    createThreadIDInitialisationStatement ();

    virtual void
    createOpDatFormalParameterDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranCUDAKernelSubroutineDirectLoop (std::string const & subroutineName,
        std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranReductionSubroutines * reductionSubroutines,
        FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclaration,
        FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration);
};

#endif
