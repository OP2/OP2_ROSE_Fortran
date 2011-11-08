#pragma once
#ifndef CPP_OPENCL_HOST_SUBROUTINE_DIRECT_LOOP_H
#define CPP_OPENCL_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <CPPOpenCLHostSubroutine.h>

class CPPOpenCLHostSubroutineDirectLoop: public CPPOpenCLHostSubroutine
{
  private:

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPOpenCLHostSubroutineDirectLoop (SgScopeStatement * moduleScope,
        CPPOpenCLKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop);
};

#endif
