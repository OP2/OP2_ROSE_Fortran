#pragma once
#ifndef CPP_REDUCTION_SUBROUTINES_H
#define CPP_REDUCTION_SUBROUTINES_H

#include <ReductionSubroutines.h>

class SgFunctionDeclaration;

class CPPReductionSubroutines: public ReductionSubroutines <
    SgFunctionDeclaration>
{
  public:

    CPPReductionSubroutines () :
      ReductionSubroutines <SgFunctionDeclaration> ()
    {
    }
};

#endif
