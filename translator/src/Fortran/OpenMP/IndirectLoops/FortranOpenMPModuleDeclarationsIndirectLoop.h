#ifndef FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_INDIRECT_LOOP_H
#define FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_INDIRECT_LOOP_H

#include <FortranOpenMPModuleDeclarations.h>

class FortranOpenMPModuleDeclarationsIndirectLoop: public FortranOpenMPModuleDeclarations
{
  public:

    FortranOpenMPModuleDeclarationsIndirectLoop (
        std::string const & userSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);

};

#endif
