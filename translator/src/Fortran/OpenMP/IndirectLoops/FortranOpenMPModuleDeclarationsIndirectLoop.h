#pragma once
#ifndef FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_INDIRECT_LOOP_H
#define FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_INDIRECT_LOOP_H

#include <FortranOpenMPModuleDeclarations.h>

class FortranOpenMPModuleDeclarationsIndirectLoop: public FortranOpenMPModuleDeclarations
{
  private:

    void
    createExecutionPlanDeclarations ();

  public:

    FortranOpenMPModuleDeclarationsIndirectLoop (
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);

};

#endif
