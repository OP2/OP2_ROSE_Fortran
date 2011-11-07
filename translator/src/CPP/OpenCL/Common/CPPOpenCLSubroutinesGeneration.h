/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Generates subroutines for CUDA Fortran
 */

#ifndef CPP_OPENCL_SUBROUTINES_GENERATION_H
#define CPP_OPENCL_SUBROUTINES_GENERATION_H

#include <CPPSubroutinesGeneration.h>
#include <CPPOpenCLDataSizesDeclaration.h>

namespace Libraries
{
  std::string const OPENCL = "CL/cl.h";
}

class CPPOpenCLSubroutinesGeneration: public CPPSubroutinesGeneration
{
  private:
  
  std::map <std::string, CPPOpenCLDataSizesDeclaration *>
      dataSizesDeclarations;

    virtual CPPHostSubroutine *
    createSubroutines ();


    virtual void
    addLibraries ();

  public:

    CPPOpenCLSubroutinesGeneration (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
