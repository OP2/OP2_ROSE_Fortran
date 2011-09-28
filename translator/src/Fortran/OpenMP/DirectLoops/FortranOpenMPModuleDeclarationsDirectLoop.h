#ifndef FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_DIRECT_LOOP_H
#define FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_DIRECT_LOOP_H

#include <FortranOpenMPModuleDeclarations.h>

class FortranOpenMPModuleDeclarationsDirectLoop: public FortranOpenMPModuleDeclarations
{
  public:

    FortranOpenMPModuleDeclarationsDirectLoop (
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);

};

#endif
