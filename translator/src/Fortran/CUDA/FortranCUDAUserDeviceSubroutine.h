#ifndef FORTRAN_CUDA_USER_DEVICE_SUBROUTINE_H
#define FORTRAN_CUDA_USER_DEVICE_SUBROUTINE_H

#include <Subroutine.h>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <ParallelLoop.h>

class FortranCUDAUserDeviceSubroutine: public Subroutine
{
  private:

    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine;

    Declarations * declarations;

    ParallelLoop * parallelLoop;

    /*
     * ======================================================
     * The name of the user subroutine on the host
     * ======================================================
     */
    std::string hostSubroutineName;

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
    createFormalParameterDeclarations ();

    void
    createLocalVariableDeclarations ();

    void
    createStatements ();

  public:

    /*
     * ======================================================
     * The name of the user subroutine on the host
     * ======================================================
     */
    std::string const &
    getHostSubroutineName ();

    FortranCUDAUserDeviceSubroutine (std::string const & subroutineName,
        FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
        Declarations * declarations, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
