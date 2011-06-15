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

    virtual SgStatement *
    createStatementToCallKernelFunction ();

    virtual void
    createStatementsToConvertCPointers (
        std::vector <SgStatement *> & statements);

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
            FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclarationOfIndirectLoop);
};

#endif
