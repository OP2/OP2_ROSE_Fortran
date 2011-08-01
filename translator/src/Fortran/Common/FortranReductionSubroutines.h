/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_REDUCTION_SUBROUTINES_H
#define FORTRAN_REDUCTION_SUBROUTINES_H

#include <ReductionSubroutines.h>

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
