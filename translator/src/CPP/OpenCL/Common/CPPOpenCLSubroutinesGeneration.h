/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Generates subroutines for CUDA Fortran
 */

#ifndef CPP_OPENCL_SUBROUTINES_GENERATION_H
#define CPP_OPENCL_SUBROUTINES_GENERATION_H

#include <CPPSubroutinesGeneration.h>

class CPPOpenCLSubroutinesGeneration: public CPPSubroutinesGeneration
{
  private:

    virtual CPPHostSubroutine *
    createSubroutines (CPPParallelLoop * parallelLoop,
        std::string const & userSubroutineName);

    virtual void
    addLibraries (SgModuleStatement * moduleStatement);

  public:

    CPPOpenCLSubroutinesGeneration (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
