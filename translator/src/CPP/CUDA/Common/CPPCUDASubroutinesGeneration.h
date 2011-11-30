#pragma once
#ifndef CPP_CUDA_SUBROUTINES_GENERATION_H
#define CPP_CUDA_SUBROUTINES_GENERATION_H

#include <CPPSubroutinesGeneration.h>

class CPPCUDASubroutinesGeneration: public CPPSubroutinesGeneration
{
  private:

    virtual void
    addFreeVariableDeclarations ();

    void
    createReductionSubroutines ();

    virtual void
    addHeaderIncludes ();

    virtual void
    createSubroutines ();

  public:

    CPPCUDASubroutinesGeneration (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations);

};

#endif
