#ifndef FORTRAN_OPENMP_KERNEL_SUBROUTINE_H
#define FORTRAN_OPENMP_KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

class FortranOpenMPKernelSubroutine: public Subroutine
{
  protected:

    Subroutine * userSubroutine;

    ParallelLoop * parallelLoop;

  protected:

    FortranOpenMPKernelSubroutine (std::string const & subroutineName,
        Subroutine * userSubroutine, ParallelLoop * parallelLoop);
};

#endif
