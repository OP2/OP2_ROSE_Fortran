#pragma once
#ifndef CPP_OPENMP_KERNEL_SUBROUTINE_H
#define CPP_OPENMP_KERNEL_SUBROUTINE_H

#include <CPPKernelSubroutine.h>

class CPPUserSubroutine;

class CPPOpenMPKernelSubroutine: public CPPKernelSubroutine
{
  protected:

    CPPOpenMPKernelSubroutine (SgScopeStatement * moduleScope,
        CPPUserSubroutine * userSubroutine, CPPParallelLoop * parallelLoop);

};

#endif
