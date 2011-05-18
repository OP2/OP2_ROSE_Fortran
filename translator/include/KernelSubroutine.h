#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <UserDeviceSubroutine.h>
#include <ParallelLoop.h>
#include <DeviceDataSizesDeclaration.h>

class KernelSubroutine: public Subroutine
{
  protected:
    /*
     * ======================================================
     * The OP_DAT formal parameters
     * ======================================================
     */

    std::map <unsigned int, SgVariableDeclaration *> formalParameter_OP_DATs;

    /*
     * ======================================================
     * Local thread variables
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_localThreadVariables;

  protected:

    void
    createArgsSizesFormalParameter (
        DeviceDataSizesDeclaration & deviceDataSizesDeclaration);

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

    SgStatement
        *
        createUserSubroutineCall (
            UserDeviceSubroutine & userDeviceSubroutine,
            SgVariableDeclaration * variable_setElementCounter,
            SgVariableDeclaration * variable_offsetInThreadBlock,
            ParallelLoop & parallelLoop,
            std::map <unsigned int, SgVariableDeclaration *> * formalParameters_GlobalToLocalMapping =
                NULL,
            std::map <unsigned int, SgVariableDeclaration *> * localVariables_nbytes =
                NULL);

    KernelSubroutine (std::string const & subroutineName) :
      Subroutine (subroutineName + SubroutineNames::kernelSuffix)
    {
    }

    KernelSubroutine (std::string const & subroutineName, std::map <
        unsigned int, SgProcedureHeaderStatement *> & _reductSubroutines) :
      Subroutine (subroutineName + SubroutineNames::kernelSuffix),
          reductionSubroutines (_reductSubroutines)
    {
    }

    /*
     * ======================================================
     * repository of SgProcedureHeaderStatements for
     * reduction subroutines (one per type), given the
     * corresponding op_dat index
     * ======================================================
     */

    std::map <unsigned int, SgProcedureHeaderStatement *> reductionSubroutines;
};

#endif
