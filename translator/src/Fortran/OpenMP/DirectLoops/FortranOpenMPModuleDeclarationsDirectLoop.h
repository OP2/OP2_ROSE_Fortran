#ifndef FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_DIRECT_LOOP_H
#define FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_DIRECT_LOOP_H

#include <FortranOpenMPModuleDeclarations.h>

class FortranOpenMPModuleDeclarationsDirectLoop: public FortranOpenMPModuleDeclarations
{
  public:

    FortranOpenMPModuleDeclarationsDirectLoop (
        std::string const & userSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);

};

#endif
