#pragma once
#ifndef FORTRAN_CUDA_USER_SUBROUTINE_H
#define FORTRAN_CUDA_USER_SUBROUTINE_H

#include <FortranUserSubroutine.h>

class FortranCUDAUserSubroutine: public FortranUserSubroutine
{
  private:

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranCUDAUserSubroutine (SgScopeStatement * moduleScope,
        FortranParallelLoop * parallelLoop,
        FortranProgramDeclarationsAndDefinitions * declarations);
};

#endif
