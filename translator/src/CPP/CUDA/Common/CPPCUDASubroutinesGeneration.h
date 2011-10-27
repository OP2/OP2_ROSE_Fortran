#pragma once
#ifndef CPP_CUDA_SUBROUTINES_GENERATION_H
#define CPP_CUDA_SUBROUTINES_GENERATION_H

#include <CPPSubroutinesGeneration.h>

class CPPCUDAReductionSubroutines;

class CPPCUDASubroutinesGeneration: public CPPSubroutinesGeneration
{
  private:

    CPPCUDAReductionSubroutines * reductionSubroutines;

  private:

    void
    createReductionSubroutines ();

    virtual void
    createSubroutines ();

  public:

    CPPCUDASubroutinesGeneration (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations);

};

#endif
