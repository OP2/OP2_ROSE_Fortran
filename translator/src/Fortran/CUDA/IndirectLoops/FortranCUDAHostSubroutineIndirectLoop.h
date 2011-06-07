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

    FortranCUDADataSizesDeclarationIndirectLoop
        * dataSizesDeclarationOfIndirectLoop;

  private:

    void
    createPlanCToForttranPointerConversionStatements ();

    void
    createExecutionPlanExecutionStatements ();

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
            std::string const & userSubroutineName,
            std::string const & kernelSubroutineName,
            FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
            FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclarationOfIndirectLoop,
            ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
