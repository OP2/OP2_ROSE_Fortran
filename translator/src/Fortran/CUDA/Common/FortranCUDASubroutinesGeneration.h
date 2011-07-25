/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Generates subroutines for CUDA Fortran
 */

#ifndef FORTRAN_CUDA_SUBROUTINES_GENERATION_H
#define FORTRAN_CUDA_SUBROUTINES_GENERATION_H

#include <FortranSubroutinesGeneration.h>

class FortranCUDASubroutinesGeneration: public FortranSubroutinesGeneration
{
  private:

    virtual FortranHostSubroutine *
    createSubroutines (FortranParallelLoop * parallelLoop,
        std::string const & userSubroutineName);

    virtual void
    addLibraries ();

  public:

    FortranCUDASubroutinesGeneration (SgProject * project,
        FortranProgramDeclarationsAndDefinitions * declarations);
};

#endif
