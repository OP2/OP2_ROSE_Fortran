/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_CUDA_MODULE_DECLARATIONS_INDIRECT_LOOP_H
#define FORTRAN_CUDA_MODULE_DECLARATIONS_INDIRECT_LOOP_H

#include <FortranCUDAModuleDeclarations.h>

class FortranCUDAModuleDeclarationsIndirectLoop: public FortranCUDAModuleDeclarations
{
  private:

  public:

    FortranCUDAModuleDeclarationsIndirectLoop (
        std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
