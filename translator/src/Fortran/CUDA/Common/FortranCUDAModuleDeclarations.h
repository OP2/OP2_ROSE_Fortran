/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_CUDA_MODULE_DECLARATIONS_H
#define FORTRAN_CUDA_MODULE_DECLARATIONS_H

#include <FortranModuleDeclarations.h>

class FortranCUDAModuleDeclarations: public FortranModuleDeclarations
{
  protected:

    void
    createOpDatSizesDeclarations ();

    FortranCUDAModuleDeclarations (std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
