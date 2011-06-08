#ifndef FORTRAN_KERNEL_SUBROUTINE_H
#define FORTRAN_KERNEL_SUBROUTINE_H

#include <KernelSubroutine.h>
#include <FortranSubroutine.h>
#include <ParallelLoop.h>

class FortranKernelSubroutine: public FortranSubroutine,
    public KernelSubroutine
{
  protected:

    FortranKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
