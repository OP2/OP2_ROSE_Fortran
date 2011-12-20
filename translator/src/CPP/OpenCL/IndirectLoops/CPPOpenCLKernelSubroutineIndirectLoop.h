#pragma once
#ifndef CPP_OPENCL_KERNEL_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENCL_KERNEL_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenCLKernelSubroutine.h>

class CPPOpenCLUserSubroutine;

class CPPOpenCLKernelSubroutineIndirectLoop: public CPPOpenCLKernelSubroutine
{
  private:

    std::map <std::string, SgVariableDeclaration *>
        indirectOpDatSharedMemoryDeclarations;

  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    SgBasicBlock *
    createIncrementAndWriteAccessEpilogueStatements ();

    SgBasicBlock *
    createStageOutFromLocalMemoryToSharedMemoryStatements ();

    SgBasicBlock *
    createInitialiseIncrementAccessStatements ();

    virtual void
    createExecutionLoopStatements ();

    SgBasicBlock *
    createInitialiseOpenCLSharedVariablesStatements ();

    SgBasicBlock *
    createSetIndirectionMapPointerStatements ();

    SgBasicBlock *
    createSetOpDatSharedMemoryPointerStatements ();

    SgBasicBlock *
    createSetNumberOfIndirectElementsPerBlockStatements ();

    SgBasicBlock *
    createIncrementAccessThreadZeroStatements ();

    SgIfStmt *
    createThreadZeroStatements ();

    virtual void
    createStatements ();

    void
    createIncrementAccessLocalVariableDeclarations ();

    void
    createStageInVariableDeclarations ();

    void
    createExecutionLocalVariableDeclarations ();

    virtual void
    createLocalVariableDeclarations ();

    void
    createPlanFormalParameterDeclarations ();

    virtual void
    createOpDatFormalParameterDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPOpenCLKernelSubroutineIndirectLoop (SgScopeStatement * moduleScope,
        CPPOpenCLUserSubroutine * userSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPReductionSubroutines * reductionSubroutines,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
