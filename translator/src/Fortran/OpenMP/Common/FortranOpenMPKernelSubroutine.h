#ifndef FORTRAN_OPENMP_KERNEL_SUBROUTINE_H
#define FORTRAN_OPENMP_KERNEL_SUBROUTINE_H

#include <FortranKernelSubroutine.h>

class FortranOpenMPKernelSubroutine: public FortranKernelSubroutine
{
  protected:

    FortranOpenMPKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
