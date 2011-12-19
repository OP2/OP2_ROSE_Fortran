#pragma once
#ifndef CPP_OPENCL_HOST_SUBROUTINE_H
#define CPP_OPENCL_HOST_SUBROUTINE_H

#include <CPPHostSubroutine.h>

class CPPOpenCLKernelSubroutine;
class CPPModuleDeclarations;

class CPPOpenCLHostSubroutine: public CPPHostSubroutine
{
  protected:

    CPPModuleDeclarations * moduleDeclarations;

  protected:

    void
    createKernelCallEpilogueStatements (SgScopeStatement * scope);

    virtual void
    createReductionEpilogueStatements ();

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionDeclarations ();

    void
    createOpenCLConfigurationLaunchDeclarations ();

    CPPOpenCLHostSubroutine (SgScopeStatement * moduleScope,
        CPPOpenCLKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPModuleDeclarations * moduleDeclarations);
};

#endif
