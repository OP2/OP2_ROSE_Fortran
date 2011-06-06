#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_H

#include <FortranKernelSubroutine.h>
#include <ParallelLoop.h>
#include <FortranCUDADataSizesDeclaration.h>

class FortranCUDAKernelSubroutine: public FortranKernelSubroutine
{
  protected:

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

    void
    createLocalThreadDeclarations ();

    void
    createAutosharedDeclaration ();

    FortranCUDAKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
