#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>
#include <FortranOpenMPHostSubroutine.h>
#include <ParallelLoop.h>

class FortranOpenMPHostSubroutine: public FortranHostSubroutine
{
  protected:

    Subroutine * kernelSubroutine;

  protected:

    virtual void
    createFormalParameterDeclarations ();

    FortranOpenMPHostSubroutine (std::string const & subroutineName,
        Subroutine * userSubroutine,
        Subroutine * kernelSubroutine,
        ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
