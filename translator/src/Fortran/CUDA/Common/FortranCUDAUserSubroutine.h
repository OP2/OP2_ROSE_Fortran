#ifndef FORTRAN_CUDA_USER_SUBROUTINE_H
#define FORTRAN_CUDA_USER_SUBROUTINE_H

#include <UserSubroutine.h>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <FortranParallelLoop.h>

class FortranCUDAUserSubroutine: public UserSubroutine <
    SgProcedureHeaderStatement, FortranProgramDeclarationsAndDefinitions>
{
  private:

    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine;

  private:

    void
    patchReferencesToConstants ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

        FortranCUDAUserSubroutine (
            std::string const & subroutineName,
            SgScopeStatement * moduleScope,
            FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
            FortranProgramDeclarationsAndDefinitions * declarations,
            FortranParallelLoop * parallelLoop);
};

#endif
