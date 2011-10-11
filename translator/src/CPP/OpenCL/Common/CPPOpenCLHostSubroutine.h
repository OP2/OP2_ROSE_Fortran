#pragma once
#ifndef CPP_OPENCL_HOST_SUBROUTINE_H
#define CPP_OPENCL_HOST_SUBROUTINE_H

#include <CPPHostSubroutine.h>

class CPPOpenCLKernelSubroutine;

class CPPOpenCLHostSubroutine: public CPPHostSubroutine
{
  protected:

    virtual void
    createReductionEpilogueStatements ();

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionLocalVariableDeclarations ();

    CPPOpenCLHostSubroutine (SgScopeStatement * moduleScope,
        CPPOpenCLKernelSubroutine * kernelSubroutine,
        CPPParallelLoop * parallelLoop);
};

#endif
