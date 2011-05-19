#ifndef HOST_SUBROUTINE_H
#define HOST_SUBROUTINE_H

#include <Subroutine.h>
#include <KernelSubroutine.h>
#include <UserDeviceSubroutine.h>
#include <ParallelLoop.h>
#include <DeviceDataSizesDeclaration.h>

class HostSubroutine: public Subroutine
{
  protected:

    /*
     * ======================================================
     * The formal parameter which a string containing the name
     * of the user device subroutine
     * ======================================================
     */
    SgVariableDeclaration * formalParameter_SubroutineName;

    /*
     * ======================================================
     * The formal OP_SET parameter
     * ======================================================
     */
    SgVariableDeclaration * formalParameter_OP_SET;

    /*
     * ======================================================
     * The formal indirection index parameters
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *>
        formalParameters_OP_INDIRECTION;

    /*
     * ======================================================
     * The formal OP_DAT parameters
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *> formalParameters_OP_DAT;

    /*
     * ======================================================
     * The formal OP_MAP parameters
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *> formalParameters_OP_MAP;

    /*
     * ======================================================
     * The formal OP_ACCESS parameters
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *> formalParameters_OP_ACCESS;

    /*
     * ======================================================
     * Local variables which store the size of OP_DAT
     * data. These allow data to be marshalled between the
     * host and the device
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_OP_DAT_Sizes;

    /*
     * ======================================================
     * Local variables which allow C pointers to be converted
     * into Fortran pointers. These allow data to be marshalled
     * between the host and the device
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_CToFortranPointers;

    /*
     * ======================================================
     * Local variables which allocate OP_DAT variables on the
     * device. These allow data to be marshalled between the
     * host and the device
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_OP_DAT_VariablesOnDevice;

    std::map <std::string, SgVariableDeclaration *> localVariables_Others;

    /*
     * ======================================================
     * Variables passed to the kernel at launch time which
     * are specific to the CUDA run-time, i.e. those between
     * triple chevrons <<< >>>
     * ======================================================
     */
    SgVariableDeclaration * CUDAVariable_blocksPerGrid;

    SgVariableDeclaration * CUDAVariable_threadsPerBlock;

    SgVariableDeclaration * CUDAVariable_sharedMemorySize;

    /*
     * ======================================================
     * Variables required to implement reductions
     * ======================================================
     */
    SgVariableDeclaration * reductionVariable_baseOffsetInSharedMemory;

    SgVariableDeclaration * reductionVariable_maxBytesInSharedMemory;

    SgVariableDeclaration * reductionVariable_numberOfThreadItems;

    SgVariableDeclaration * reductionVariable_maximumNumberOfThreadBlocks;

    SgVariableDeclaration * reductionVariable_reductionArrayOnHost;

    SgVariableDeclaration * reductionVariable_reductionArrayOnDevice;

  protected:

    void
    copyDataBackFromDeviceAndDeallocate (ParallelLoop & parallelLoop);

    void
    initialiseDataMarshallingLocalVariables (ParallelLoop & parallelLoop);

    void
    createDataMarshallingLocalVariables (ParallelLoop & parallelLoop);

    void
    createCUDAKernelVariables ();

    void
    createFormalParameters (UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop);

    SgStatement *
    createCallToC_F_POINTER (SgExpression * parameter1,
        SgExpression * parameter2, SgExpression * parameter3 = NULL);

    SgStatement *
    buildThreadSynchroniseFunctionCall (SgScopeStatement * subroutineScope);

    void
    createReductionVariables (ParallelLoop & parallelLoop);

    void
    createAndAppendThreadSynchCall ();

    void
        createSupportForReductionVariablesBeforeKernel (
            ParallelLoop & parallelLoop);

    void
    createSupportForReductionVariablesAfterKernel (ParallelLoop & parallelLoop);

    void
    createAndAppendIterationVariablesForReduction (ParallelLoop & parallelLoop);

    HostSubroutine (std::string const & subroutineName,
        UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);

  public:

    SgVariableDeclaration *
    get_OP_DAT_VariableOnDevice (unsigned int OP_DAT_ArgumentGroup)
    {
      return localVariables_OP_DAT_VariablesOnDevice[OP_DAT_ArgumentGroup];
    }
};

#endif