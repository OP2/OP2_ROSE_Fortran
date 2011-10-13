#pragma once
#ifndef CPP_OPENCL_SUBROUTINES_GENERATION_H
#define CPP_OPENCL_SUBROUTINES_GENERATION_H

#include <CPPSubroutinesGeneration.h>

class CPPOpenCLSubroutinesGeneration: public CPPSubroutinesGeneration
{
  private:

    virtual CPPHostSubroutine *
    createSubroutines ();

    virtual void
    addLibraries ();

  public:

    CPPOpenCLSubroutinesGeneration (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
