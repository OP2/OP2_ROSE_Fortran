#pragma once
#ifndef FORTRAN_CUDA_USER_SUBROUTINE_H
#define FORTRAN_CUDA_USER_SUBROUTINE_H

#include <FortranUserSubroutine.h>

class FortranCUDAConstantDeclarations;

class FortranCUDAUserSubroutine: public FortranUserSubroutine
{
  public:

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

    virtual void appendAdditionalSubroutines ( SgScopeStatement * moduleScope,
      FortranParallelLoop * parallelLoop, FortranProgramDeclarationsAndDefinitions * declarations,
      FortranCUDAConstantDeclarations * CUDAconstants);

    FortranCUDAUserSubroutine (SgScopeStatement * moduleScope,
        FortranParallelLoop * parallelLoop,
        FortranProgramDeclarationsAndDefinitions * declarations);
        
    FortranCUDAUserSubroutine (SgScopeStatement * moduleScope,
        FortranParallelLoop * parallelLoop,
        FortranProgramDeclarationsAndDefinitions * declarations,
        string subroutineName);

};

#endif
