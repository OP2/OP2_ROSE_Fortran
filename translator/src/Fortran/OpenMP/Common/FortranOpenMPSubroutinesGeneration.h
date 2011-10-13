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
      FortranSubroutinesGeneration (project, declarations,
          "rose_openmp_code.F95")
    {
      generate ();
    }
};

#endif
