#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <UserDeviceSubroutine.h>
#include <ParallelLoop.h>
#include <DataSizesDeclaration.h>

class KernelSubroutine: public Subroutine
{
  protected:

    UserDeviceSubroutine * userDeviceSubroutine;

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
    createLocalThreadVariables ();

    void
    createAutosharedVariable ();

    void
    initialiseLocalThreadVariables ();

    void
    createAndAppendReductionSubroutineCall ();

    void
    createAndAppendSharedMemoryOffesetForReduction ();

    void
        createArgsSizesFormalParameter (
            DataSizesDeclaration * dataSizesDeclaration);

    SgStatement *
    createUserSubroutineCall (UserDeviceSubroutine * userDeviceSubroutine,
        SgVariableDeclaration * variable_setElementCounter,
        SgVariableDeclaration * variable_offsetInThreadBlock,
        ParallelLoop * parallelLoop);

    KernelSubroutine (std::string const & subroutineName,
        UserDeviceSubroutine * userDeviceSubroutine,
        ParallelLoop * parallelLoop);
};

#endif
