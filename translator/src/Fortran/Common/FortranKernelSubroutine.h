#ifndef FORTRAN_KERNEL_SUBROUTINE_H
#define FORTRAN_KERNEL_SUBROUTINE_H

#include <FortranSubroutine.h>
#include <ParallelLoop.h>

class FortranKernelSubroutine: public FortranSubroutine
{
  protected:

    std::string userSubroutineName;

    ParallelLoop * parallelLoop;

  protected:

    FortranKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop);
};

#endif
