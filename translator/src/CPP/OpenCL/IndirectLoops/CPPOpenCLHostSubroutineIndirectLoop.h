#pragma once
#ifndef CPP_OPENCL_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENCL_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenCLHostSubroutine.h>

class CPPOpenCLKernelSubroutineIndirectLoop;

class CPPOpenCLHostSubroutineIndirectLoop: public CPPOpenCLHostSubroutine
{

  private:

    virtual void
    createKernelFunctionCallStatement (SgScopeStatement * scope);

    SgBasicBlock *
    createPlanFunctionExecutionStatements ();

    SgStatement *
    createPlanFunctionCallStatement ();

    virtual void
    createStatements ();

    void
    createPlanFunctionDeclarations ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPOpenCLHostSubroutineIndirectLoop (SgScopeStatement * moduleScope,
        CPPOpenCLKernelSubroutine * kernelSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPModuleDeclarations * moduleDeclarations,
        CPPUserSubroutine * userSubroutine,
        CPPOpenCLConstantDeclarations * constantDeclarations);
};

#endif
