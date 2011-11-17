#pragma once
#ifndef FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_H
#define FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_H

#include "FortranModuleDeclarations.h"

class FortranOpenMPModuleDeclarations: public FortranModuleDeclarations
{
  protected:

    FortranOpenMPModuleDeclarations (FortranParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
