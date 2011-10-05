#pragma once
#ifndef FORTRAN_KERNEL_SUBROUTINE_H
#define FORTRAN_KERNEL_SUBROUTINE_H

#include <KernelSubroutine.h>

class SgProcedureHeaderStatement;
class SgScopeStatement;
class FortranParallelLoop;

class FortranKernelSubroutine: public KernelSubroutine <
    SgProcedureHeaderStatement>
{
  protected:

    FortranKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};
#endif

