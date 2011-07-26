/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Generates subroutines for CUDA Fortran
 */

#ifndef FORTRAN_CUDA_SUBROUTINES_GENERATION_H
#define FORTRAN_CUDA_SUBROUTINES_GENERATION_H

#include <FortranSubroutinesGeneration.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranCUDADataSizesDeclaration.h>
#include <FortranCUDAModuleDeclarations.h>

class FortranCUDASubroutinesGeneration: public FortranSubroutinesGeneration
{

  private:

    std::map <std::string, FortranOpDatDimensionsDeclaration *>
        dimensionsDeclarations;

    std::map <std::string, FortranCUDADataSizesDeclaration *>
        dataSizesDeclarations;

    std::map <std::string, FortranCUDAModuleDeclarations *> moduleDeclarations;

  private:

    virtual FortranHostSubroutine *
    createSubroutines (FortranParallelLoop * parallelLoop,
        std::string const & userSubroutineName);

    virtual void
    createModuleDeclarations ();

    virtual void
    addLibraries ();

  public:

    FortranCUDASubroutinesGeneration (SgProject * project,
        FortranProgramDeclarationsAndDefinitions * declarations);
};

#endif
