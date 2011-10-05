/*
 * Written by Adam Betts and Carlo Bertolli
 */

#pragma once
#ifndef FORTRAN_REDUCTION_SUBROUTINES_H
#define FORTRAN_REDUCTION_SUBROUTINES_H

#include <ReductionSubroutines.h>

class SgProcedureHeaderStatement;

class FortranReductionSubroutines: public ReductionSubroutines <
    SgProcedureHeaderStatement>
{
  public:

    FortranReductionSubroutines () :
      ReductionSubroutines <SgProcedureHeaderStatement> ()
    {
    }
};

#endif
