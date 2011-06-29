#ifndef FORTRAN_CUDA_USER_SUBROUTINE_H
#define FORTRAN_CUDA_USER_SUBROUTINE_H

#include <UserSubroutine.h>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <ParallelLoop.h>

class FortranCUDAUserSubroutine: public UserSubroutine <
    SgProcedureHeaderStatement>
{
  private:

    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine;

    Declarations <SgProcedureHeaderStatement *> * declarations;

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
            Declarations <SgProcedureHeaderStatement *> * declarations,
            ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
