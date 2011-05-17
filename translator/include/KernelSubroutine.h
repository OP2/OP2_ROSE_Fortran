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
     * The first formal parameter of the CUDA kernel in both
     * direct and indirect loops is a
     * variable containing the size information of the other
     * formal parameters
     * ======================================================
     */

    SgVariableDeclaration * formalParameter_argsSizes;

    /*
     * ======================================================
     * The OP_DAT formal parameters
     * ======================================================
     */

    std::map <unsigned int, SgVariableDeclaration *> formalParameter_OP_DATs;

    /*
     * ======================================================
     * Offset in shared memory for reduction variables,
     * in case it is needed
     * ======================================================
     */
    SgVariableDeclaration * formalParameter_offsetForReduction;

    /*
     * ======================================================
     * Size of the warp for OP2 when implementing direct
     * loops or reduction variables.
     * This value must be passed to CUDA kernels of
     * direct loops because the access to the related
     * intrinsic CUDA Fortran variable gives place to an
     * error.
     * It is also used in reduction subroutines both in
     * diret and indirect loops
     * ======================================================
     */

    SgVariableDeclaration * formalParameter_warpSizeOP2;

    /*
     * ======================================================
     * Local thread variables
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_localThreadVariables;

    /*
     * ======================================================
     * autoshared variable
     * ======================================================
     */

    SgVariableDeclaration * localVariables_autoshared;

    std::map <std::string, SgVariableDeclaration *> localVariables_Others;

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
    createLocalThreadVariables (ParallelLoop & parallelLoop,
        SgScopeStatement & scopeStatement, bool isDirectLoop);

    /*
     * ======================================================
     * Creates the autoshared variable: only for real(8)
     * ======================================================
     */

    void
    createAutosharedVariable (ParallelLoop & parallelLoop,
        SgScopeStatement * scopeStatement);

    /*
     * ======================================================
     * Initialises local thread variables
     * ======================================================
     */

    void
    initialiseLocalThreadVariables (ParallelLoop & parallelLoop,
        SgScopeStatement * scopeStatement,
        SgVarRefExp * iterationVariableReference);

    /*
     * ======================================================
     * Support for reduction variables
     * ======================================================
     */

    void
    createAndAppendReductionSubroutineCall (ParallelLoop & parallelLoop,
        SgVarRefExp * iterationVarRef, SgScopeStatement * scopeStatement);

    /*
     * ======================================================
     * Generation of formal parameter for offset in shared
     * memory reserved to reductions
     * ======================================================
     */

    void
        createAndAppendSharedMemoryOffesetForReduction (
            ParallelLoop & parallelLoop);

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
      Subroutine (subroutineName +  SubroutineNames::kernelSuffix), reductionSubroutines (
          _reductSubroutines)
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
