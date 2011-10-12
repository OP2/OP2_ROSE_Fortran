/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Generates subroutines for CUDA Fortran
 */

#pragma once
#ifndef FORTRAN_CUDA_SUBROUTINES_GENERATION_H
#define FORTRAN_CUDA_SUBROUTINES_GENERATION_H

#include <FortranSubroutinesGeneration.h>

class FortranCUDAOpDatCardinalitiesDeclaration;

class FortranCUDASubroutinesGeneration: public FortranSubroutinesGeneration
{

  private:

    std::map <std::string, FortranCUDAOpDatCardinalitiesDeclaration *>
        cardinalitiesDeclarations;

  private:

    void
    createConstantDeclarations ();

    void
    createReductionSubroutines ();

    virtual void
    createSubroutines ();

    virtual void
    createModuleDeclarations ();

    virtual void
    addLibraries ();

  public:

    FortranCUDASubroutinesGeneration (SgProject * project,
        FortranProgramDeclarationsAndDefinitions * declarations) :
      FortranSubroutinesGeneration (declarations, "rose_cuda_code.F95", project)
    {
      generate ();
    }
};

#endif
