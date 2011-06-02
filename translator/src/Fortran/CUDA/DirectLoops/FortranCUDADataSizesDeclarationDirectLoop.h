/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Models the data sizes declaration needed for a direct loop.
 * Its fields include the sizes of the OP_DAT arguments
 */

#ifndef FORTRAN_CUDA_DATA_SIZES_DECLARATION_DIRECT_LOOP_H
#define FORTRAN_CUDA_DATA_SIZES_DECLARATION_DIRECT_LOOP_H

#include <FortranCUDADataSizesDeclaration.h>

class FortranCUDADataSizesDeclarationDirectLoop: public FortranCUDADataSizesDeclaration
{
  private:

    virtual void
    addFields ();

  public:

    FortranCUDADataSizesDeclarationDirectLoop (std::string const & subroutineName,
        ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
