#ifndef FORTRAN_CUDA_USER_SUBROUTINE_H
#define FORTRAN_CUDA_USER_SUBROUTINE_H

#include <UserSubroutine.h>
#include <FortranSubroutine.h>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <ParallelLoop.h>

class FortranCUDAUserSubroutine: public UserSubroutine,
    public FortranSubroutine
{
  private:

    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine;

    Declarations * declarations;

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
    createlocalVariableDeclarations ();

    void
    createLocalVariableDeclarations ();

    void
    createStatements ();

  public:

        FortranCUDAUserSubroutine (
            std::string const & subroutineName,
            FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
            Declarations * declarations, ParallelLoop * parallelLoop,
            SgScopeStatement * moduleScope);
};

#endif
