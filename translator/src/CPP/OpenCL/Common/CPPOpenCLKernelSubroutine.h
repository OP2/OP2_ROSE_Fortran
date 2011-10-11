#pragma once
#ifndef CPP_OPENCL_KERNEL_SUBROUTINE_H
#define CPP_OPENCL_KERNEL_SUBROUTINE_H

#include <CPPKernelSubroutine.h>

class CPPReductionSubroutines;

class CPPOpenCLKernelSubroutine: public CPPKernelSubroutine
{
  protected:

    CPPOpenCLKernelSubroutine (SgScopeStatement * moduleScope, Subroutine <
        SgFunctionDeclaration> * userSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPReductionSubroutines * reductionSubroutines);
};

#endif
