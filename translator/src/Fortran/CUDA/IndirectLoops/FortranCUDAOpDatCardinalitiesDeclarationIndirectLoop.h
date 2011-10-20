#pragma once
#ifndef FORTRAN_CUDA_DATA_SIZES_DECLARATION_INDIRECT_LOOP_H
#define FORTRAN_CUDA_DATA_SIZES_DECLARATION_INDIRECT_LOOP_H

#include <FortranCUDAOpDatCardinalitiesDeclaration.h>

class FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop: public FortranCUDAOpDatCardinalitiesDeclaration
{
  private:

    void
    addIndirectLoopFields ();

  public:

    FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop (
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
