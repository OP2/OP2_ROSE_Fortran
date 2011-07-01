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

    /*
     * ======================================================
     * Copies the user function (launched by the kernel) and
     * applies some modifications so that it can run on the
     * device
     * ======================================================
     */
    void
    copyAndModifySubroutine ();

    void
    createLocalVariableDeclarations ();

    void
    createFormalParameterDeclarations ();

    void
    createStatements ();

  public:

        FortranCUDAUserSubroutine (
            std::string const & subroutineName,
            FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
            FortranProgramDeclarationsAndDefinitions * declarations,
            FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
