#pragma once
#ifndef CPP_CUDA_REDUCTION_SUBROUTINES_H
#define CPP_CUDA_REDUCTION_SUBROUTINES_H

#include <ReductionSubroutines.h>

class SgFunctionDeclaration;

class CPPCUDAReductionSubroutines: public ReductionSubroutines <
    SgFunctionDeclaration>
{
  public:

    CPPCUDAReductionSubroutines () :
      ReductionSubroutines <SgFunctionDeclaration> ()
    {
    }
};

#endif
