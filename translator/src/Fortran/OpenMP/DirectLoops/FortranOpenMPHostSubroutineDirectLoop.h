#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <FortranOpenMPHostSubroutine.h>

class FortranOpenMPHostSubroutineDirectLoop: public FortranOpenMPHostSubroutine
{
  public:

    FortranOpenMPHostSubroutineDirectLoop (std::string const & subroutineName,
        Subroutine * userSubroutine, Subroutine * kernelSubroutine,
        ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
