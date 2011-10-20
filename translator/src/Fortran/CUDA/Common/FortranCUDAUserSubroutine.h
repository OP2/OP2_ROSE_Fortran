#pragma once
#ifndef FORTRAN_CUDA_USER_SUBROUTINE_H
#define FORTRAN_CUDA_USER_SUBROUTINE_H

#include <UserSubroutine.h>

class FortranParallelLoop;
class FortranProgramDeclarationsAndDefinitions;

class FortranCUDAUserSubroutine: public UserSubroutine <
    SgProcedureHeaderStatement, FortranProgramDeclarationsAndDefinitions>
{
  private:

    void
    forceOutputOfCodeToFile ();

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
