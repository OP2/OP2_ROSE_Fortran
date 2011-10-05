/*
 * Written by Adam Betts and Carlo Bertolli
 */

#pragma once
#ifndef FORTRAN_OPENMP_SUBROUTINES_GENERATION_H
#define FORTRAN_OPENMP_SUBROUTINES_GENERATION_H

#include <FortranSubroutinesGeneration.h>

class FortranOpenMPSubroutinesGeneration: public FortranSubroutinesGeneration
{
  private:

    virtual void
    createReductionSubroutines ();

    virtual void
    createSubroutines ();

    virtual void
    createModuleDeclarations ();

    virtual void
    addLibraries ();

  public:

    FortranOpenMPSubroutinesGeneration (SgProject * project,
        FortranProgramDeclarationsAndDefinitions * declarations) :
      FortranSubroutinesGeneration (declarations, "openmp_subroutines.F95")
    {
      generate();
    }
};

#endif
