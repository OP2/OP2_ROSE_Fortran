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
#include <FortranPlan.h>

class FortranCUDAHostSubroutineIndirectLoop: public FortranCUDAHostSubroutine,
    public FortranPlan
{
  private:

    FortranCUDADataSizesDeclarationIndirectLoop
        * dataSizesDeclarationOfIndirectLoop;

  private:

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    void
    createPlanFunctionExecutionStatements ();

    void
    createVariablesSizesInitialisationStatements ();

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
            ParallelLoop * parallelLoop,
            SgScopeStatement * moduleScope,
            FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
            FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclarationOfIndirectLoop,
            FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration);
};

#endif
