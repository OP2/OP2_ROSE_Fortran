#pragma once
#ifndef CPP_OPENCL_KERNEL_SUBROUTINE_DIRECT_LOOP_H
#define CPP_OPENCL_KERNEL_SUBROUTINE_DIRECT_LOOP_H

#include <CPPOpenCLKernelSubroutine.h>

class CPPOpenCLKernelSubroutineDirectLoop: public CPPOpenCLKernelSubroutine
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

    CPPOpenCLKernelSubroutineDirectLoop (SgScopeStatement * moduleScope,
        CPPOpenCLUserSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPReductionSubroutines * reductionSubroutines);
};

#endif
