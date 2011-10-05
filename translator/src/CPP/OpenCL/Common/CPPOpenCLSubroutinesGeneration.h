#pragma once
#ifndef CPP_OPENCL_SUBROUTINES_GENERATION_H
#define CPP_OPENCL_SUBROUTINES_GENERATION_H

#include <CPPSubroutinesGeneration.h>

class CPPOpenCLDataSizesDeclaration;

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
