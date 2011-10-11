#pragma once
#ifndef CPP_OPENCL_KERNEL_SUBROUTINE_DIRECT_LOOP_H
#define CPP_OPENCL_KERNEL_SUBROUTINE_DIRECT_LOOP_H

#include <CPPOpenCLKernelSubroutine.h>

class CPPOpenCLUserSubroutine;

class CPPOpenCLKernelSubroutineDirectLoop: public CPPOpenCLKernelSubroutine
{

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

    void
    createInitialiseLocalThreadVariablesStatements ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPOpenCLKernelSubroutineDirectLoop (SgScopeStatement * moduleScope,
        CPPOpenCLUserSubroutine * userSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPReductionSubroutines * reductionSubroutines);
};

#endif
