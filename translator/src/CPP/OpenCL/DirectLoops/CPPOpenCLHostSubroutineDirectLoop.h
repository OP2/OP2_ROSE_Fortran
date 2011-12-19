#pragma once
#ifndef CPP_OPENCL_HOST_SUBROUTINE_DIRECT_LOOP_H
#define CPP_OPENCL_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <CPPOpenCLHostSubroutine.h>

class CPPOpenCLHostSubroutineDirectLoop: public CPPOpenCLHostSubroutine
{
  private:

    void
    createOpenCLKernelInitialisationStatements ();

    void
    createOpenCLKernelActualParameterDeclarations ();

    virtual void
    createKernelFunctionCallStatement (SgScopeStatement * scope);

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPOpenCLHostSubroutineDirectLoop (SgScopeStatement * moduleScope,
        CPPOpenCLKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPModuleDeclarations * moduleDeclarations);
};

#endif
