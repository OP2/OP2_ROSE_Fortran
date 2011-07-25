/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_OPENMP_SUBROUTINES_GENERATION_H
#define FORTRAN_OPENMP_SUBROUTINES_GENERATION_H

#include <FortranSubroutinesGeneration.h>

class FortranOpenMPSubroutinesGeneration: public FortranSubroutinesGeneration
{
  private:

    virtual FortranHostSubroutine *
    createSubroutines (FortranParallelLoop * parallelLoop,
        std::string const & userSubroutineName);

    virtual void
    addLibraries ();

  public:

    FortranOpenMPSubroutinesGeneration (SgProject * project,
        FortranProgramDeclarationsAndDefinitions * declarations);
};

#endif
