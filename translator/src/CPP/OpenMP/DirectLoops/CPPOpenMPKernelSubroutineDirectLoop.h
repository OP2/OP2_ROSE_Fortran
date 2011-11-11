#pragma once
#ifndef CPP_OPENMP_KERNEL_SUBROUTINE_DIRECT_LOOP_H
#define CPP_OPENMP_KERNEL_SUBROUTINE_DIRECT_LOOP_H

#include <CPPOpenMPKernelSubroutine.h>

class CPPOpenMPKernelSubroutineDirectLoop: public CPPOpenMPKernelSubroutine
{
  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    virtual void
    createExecutionLoopStatements ();

    virtual void
    createOpDatFormalParameterDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPOpenMPKernelSubroutineDirectLoop (SgScopeStatement * moduleScope,
        CPPUserSubroutine * userSubroutine, CPPParallelLoop * parallelLoop);
};

#endif
