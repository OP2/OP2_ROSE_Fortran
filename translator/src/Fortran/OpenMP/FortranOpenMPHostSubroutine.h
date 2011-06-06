#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>
#include <FortranOpenMPModuleDeclarations.h>
#include <ParallelLoop.h>

class FortranOpenMPHostSubroutine: public FortranHostSubroutine
{
  protected:

    FortranOpenMPModuleDeclarations * moduleDeclarations;

  protected:

    FortranOpenMPHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope,
        FortranOpenMPModuleDeclarations * moduleDeclarations);
};

#endif
