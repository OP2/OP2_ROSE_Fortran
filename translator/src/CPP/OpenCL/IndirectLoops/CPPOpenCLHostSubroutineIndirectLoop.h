#pragma once
#ifndef CPP_OPENCL_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENCL_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenCLHostSubroutine.h>
#include <CPPPlan.h>

class CPPOpenCLKernelSubroutineIndirectLoop;

class CPPOpenCLHostSubroutineIndirectLoop: public CPPOpenCLHostSubroutine,
    public CPPPlan
{
  private:

    void
    createPlanFunctionExecutionStatements ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    void
    createExecutionPlanDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPOpenCLHostSubroutineIndirectLoop (SgScopeStatement * moduleScope,
        CPPOpenCLKernelSubroutine * kernelSubroutine,
        CPPParallelLoop * parallelLoop);
};

#endif
