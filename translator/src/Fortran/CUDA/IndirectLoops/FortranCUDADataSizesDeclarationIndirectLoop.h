/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_CUDA_DATA_SIZES_DECLARATION_INDIRECT_LOOP_H
#define FORTRAN_CUDA_DATA_SIZES_DECLARATION_INDIRECT_LOOP_H

#include <FortranCUDADataSizesDeclaration.h>

class FortranCUDADataSizesDeclarationIndirectLoop: public FortranCUDADataSizesDeclaration
{
  private:

    virtual void
    addFields ();

  public:

    FortranCUDADataSizesDeclarationIndirectLoop (
        std::string const & subroutineName, FortranParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
