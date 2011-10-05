/*
 * Written by Adam Betts and Carlo Bertolli
 */

#pragma once
#ifndef CPP_REDUCTION_SUBROUTINES_H
#define CPP_REDUCTION_SUBROUTINES_H

#include <ReductionSubroutines.h>

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
