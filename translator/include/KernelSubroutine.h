#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <boost/lexical_cast.hpp>
#include <Subroutine.h>
#include <UserDeviceSubroutine.h>
#include <ParallelLoop.h>
#include <DataSizesDeclaration.h>

class KernelSubroutine: public Subroutine
{
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
        createArgsSizesFormalParameter (
            DataSizesDeclaration & dataSizesDeclaration);

    /*
     * ======================================================
     * Creates the argI_l local thread variables
     * for CUDA kernels of direct and indirect loops
     * ======================================================
     */

    void
    createLocalThreadVariables (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Creates the autoshared variable: only for real(8)
     * ======================================================
     */
    void
    createAutosharedVariable (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Initialises local thread variables
     * ======================================================
     */
    void
    initialiseLocalThreadVariables (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Support for reduction variables
     * ======================================================
     */
    void
    createAndAppendReductionSubroutineCall (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Generation of formal parameter for offset in shared
     * memory reserved to reductions
     * ======================================================
     */
    void
    createAndAppendSharedMemoryOffesetForReduction ();

    SgStatement *
    createUserSubroutineCall (UserDeviceSubroutine & userDeviceSubroutine,
        SgVariableDeclaration * variable_setElementCounter,
        SgVariableDeclaration * variable_offsetInThreadBlock,
        ParallelLoop & parallelLoop);

    KernelSubroutine (std::string const & subroutineName) :
      Subroutine (subroutineName + SubroutineNameSuffixes::kernelSuffix)
    {
    }
};

#endif
