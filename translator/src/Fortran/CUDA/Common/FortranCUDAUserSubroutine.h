#pragma once
#ifndef FORTRAN_CUDA_USER_SUBROUTINE_H
#define FORTRAN_CUDA_USER_SUBROUTINE_H

#include <FortranUserSubroutine.h>

class FortranCUDAConstantDeclarations;

class FortranCUDAUserSubroutine: public FortranUserSubroutine
{
  private:
  
    /*
     * ======================================================
     * This boolean indicates if the user subroutine is one
     * used as an argument to an op_par_loop or if it is 
     * called inside a user kernel or another user subroutine
     * It permits handling the "value" attribute in a 
     * different way w.r.t. the parallelLoop object in which
     * it is called and the positional identities of its
     * formal parameters
     * ======================================================
     */
    bool isUserKernel;

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
