/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the host subroutine for an indirect loop.
 * Its declarations and statements do the following:
 */

#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <FortranCUDAHostSubroutine.h>
#include <FortranCUDAKernelSubroutine.h>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <FortranCUDADataSizesDeclarationIndirectLoop.h>

class FortranCUDAHostSubroutineIndirectLoop: public FortranCUDAHostSubroutine
{
  private:

    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine;

    FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclarationOfIndirectLoop;

  private:

    /*
     * ======================================================
     * Returns the name of the local variable which models
     * the mappings from local indices to global indices in
     * shared memory
     * ======================================================
     */
    static std::string
    getLocalToGlobalMappingVariableName (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Returns the name of the local variable which models
     * the indirect mappings to local indices in shared memory
     * ======================================================
     */
    static std::string
    getGlobalToLocalMappingVariableName (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Returns the name of the local variable which models
     * the number of indirect elements in each block
     * ======================================================
     */
    static std::string
    getGlobalToLocalMappingSizeVariableName (unsigned int OP_DAT_ArgumentGroup);

    void
    createExecutionPlanExecutionStatements ();

    void
    createPlanCToForttranPointerConversionStatements ();

    void
    createPlanFunctionCallStatement ();

    void
    createDoLoopToCorrectIndexing ();

    void
    createExecutionPlanStatements ();

    void
    initialiseDeviceVariablesSizesVariable ();

    void
    initialiseVariablesAndConstants ();

    void
    createExecutionPlanDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

        FortranCUDAHostSubroutineIndirectLoop (
            std::string const & subroutineName,
            FortranCUDAUserDeviceSubroutine * userDeviceSubroutine,
            FortranCUDAKernelSubroutine * kernelSubroutine,
            FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
            FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclarationOfIndirectLoop,
            ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
