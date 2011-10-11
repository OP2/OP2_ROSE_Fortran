#pragma once
#ifndef FORTRAN_OPENMP_KERNEL_SUBROUTINE_H
#define FORTRAN_OPENMP_KERNEL_SUBROUTINE_H

#include <FortranKernelSubroutine.h>

class FortranOpenMPKernelSubroutine: public FortranKernelSubroutine
{
  protected:

    FortranOpenMPKernelSubroutine (SgScopeStatement * moduleScope, Subroutine <
        SgProcedureHeaderStatement> * userSubroutine,
        FortranParallelLoop * parallelLoop) :
      FortranKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
    {
    }
};

#endif
