#pragma once
#ifndef CPP_OPENCL_SUBROUTINES_GENERATION_H
#define CPP_OPENCL_SUBROUTINES_GENERATION_H

#include <CPPSubroutinesGeneration.h>

class CPPOpenCLSubroutinesGeneration: public CPPSubroutinesGeneration
{
  private:

    virtual void
    addFreeVariableDeclarations ();

    void
    createReductionSubroutines ();

    void
    addHeaderIncludes ();

    virtual void
    createSubroutines ();

  public:

    CPPOpenCLSubroutinesGeneration (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
