/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_CUDA_MODULE_DECLARATIONS_DIRECT_LOOP_H
#define FORTRAN_CUDA_MODULE_DECLARATIONS_DIRECT_LOOP_H

#include <FortranCUDAModuleDeclarations.h>
#include <FortranCUDADataSizesDeclarationDirectLoop.h>

class FortranCUDAModuleDeclarationsDirectLoop: public FortranCUDAModuleDeclarations
{
  private:

  public:

    FortranCUDAModuleDeclarationsDirectLoop (
        std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclaration);
};

#endif
