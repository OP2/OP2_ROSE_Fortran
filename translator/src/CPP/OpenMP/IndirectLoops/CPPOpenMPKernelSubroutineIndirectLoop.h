#pragma once
#ifndef CPP_OPENMP_KERNEL_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENMP_KERNEL_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenMPKernelSubroutine.h>

class CPPOpenMPKernelSubroutineIndirectLoop: public CPPOpenMPKernelSubroutine
{
  private:

    std::map <std::string, SgVariableDeclaration *>
        sharedIndirectionDeclarations;

  private:

    SgBasicBlock *
    createStageOutIncrementedOpDatStatements ();

    SgBasicBlock *
    createIncrementedOpDatPrologueStatements ();

    void
    createIncrementedOpDatEpilogueStatements ();

    void
    createStageInStatements ();

    void
    createInitialiseSharedVariableStatements ();

    void
    createInitialiseThreadVariablesStatements ();

    void
    createInitialiseIncrementAccessVariablesStatements ();

    void
    createInitialiseIndirectOpDatSizesStatements ();

    void
    createInitialiseIndirectOpDatMapsStatements ();

    void
    createIncrementAccessLocalVariableDeclarations ();

    void
    createIndirectOpDatSizeLocalVariableDeclarations ();

    void
    createSharedVariableDeclarations ();

    void
    createExecutionLocalVariableDeclarations ();

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    virtual void
    createExecutionLoopStatements ();

    void
    createPlanFormalParameterDeclarations ();

    virtual void
    createOpDatFormalParameterDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPOpenMPKernelSubroutineIndirectLoop (SgScopeStatement * moduleScope,
        CPPUserSubroutine * userSubroutine, CPPParallelLoop * parallelLoop);
};

#endif
