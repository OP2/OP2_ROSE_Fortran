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

    void
    createOPDATFormalParameterDeclarations ();

    /*
     * ======================================================
     * Builds the statements included in the main
     * set elements-based loop
     * ======================================================
     */
    SgBasicBlock *
    buildMainLoopStatements ();

    /*
     * ======================================================
     * Builds the assignments of shared memory and local
     * thread variables, if needed
     * ======================================================
     */
    SgBasicBlock *
    stageInFromDeviceMemoryToLocalThreadVariables ();

    /*
     * ======================================================
     * Builds the assignments of shared memory and local
     * thread variables, if needed
     * ======================================================
     */
    SgBasicBlock *
    stageOutFromLocalThreadVariablesToDeviceMemory ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranCUDAKernelSubroutineDirectLoop (std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope,
        FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclaration);
};

#endif
