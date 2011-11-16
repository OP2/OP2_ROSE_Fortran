#pragma once
#ifndef CPP_OPENMP_KERNEL_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENMP_KERNEL_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenMPKernelSubroutine.h>

class CPPOpenMPKernelSubroutineIndirectLoop: public CPPOpenMPKernelSubroutine
{
  private:

    std::map <std::string, SgVariableDeclaration *>
        indirectOpDatSharedMemoryDeclarations;

  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    virtual void
    createExecutionLoopStatements ();

    void
    createIncrementAccessLocalVariableDeclarations ();

    void
    createExecutionLocalVariableDeclarations ();

    void
    createCUDAStageInVariablesVariableDeclarations ();

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
