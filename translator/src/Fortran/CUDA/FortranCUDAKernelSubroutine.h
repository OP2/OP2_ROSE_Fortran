#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <FortranCUDAUserDeviceSubroutine.h>
#include <ParallelLoop.h>
#include <FortranCUDADataSizesDeclaration.h>

class FortranCUDAKernelSubroutine: public Subroutine
{
  protected:

    FortranCUDAUserDeviceSubroutine * userDeviceSubroutine;

    ParallelLoop * parallelLoop;

  protected:

    /*
     * ======================================================
     * Returns the name of the OP_DAT variable in this OP_DAT
     * argument group
     * ======================================================
     */
    static std::string
    get_OP_DAT_VariableName (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Returns the name of the variable modelling the size of
     * an OP_DAT in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_DAT_SizeVariableName (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Returns the name of the local thread OP_DAT variable
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    getLocalThread_OP_DAT_VariableName (unsigned int OP_DAT_ArgumentGroup);

    void
    initialiseLocalThreadVariables ();

    void
    createReductionSubroutineCall ();

    SgStatement *
    createUserSubroutineCall (FortranCUDAUserDeviceSubroutine * userDeviceSubroutine,
        SgVariableDeclaration * variable_setElementCounter,
        SgVariableDeclaration * variable_offsetInThreadBlock,
        ParallelLoop * parallelLoop);

    void
    createLocalThreadDeclarations ();

    void
    createAutosharedDeclaration ();

    FortranCUDAKernelSubroutine (std::string const & subroutineName,
        FortranCUDAUserDeviceSubroutine * userDeviceSubroutine,
        ParallelLoop * parallelLoop);
};

#endif
