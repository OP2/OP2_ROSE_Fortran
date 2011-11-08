#pragma once
#ifndef CPP_OPENCL_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENCL_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenCLHostSubroutine.h>

class CPPPlan;
class CPPOpenCLKernelSubroutineIndirectLoop;

class CPPOpenCLHostSubroutineIndirectLoop: public CPPOpenCLHostSubroutine
{
  private:

    CPPPlan * plan;

  private:

    virtual SgStatement *
    createKernelFunctionCallStatement ();

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
