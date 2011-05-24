#ifndef HOST_SUBROUTINE_H
#define HOST_SUBROUTINE_H

#include <Subroutine.h>
#include <KernelSubroutine.h>
#include <UserDeviceSubroutine.h>
#include <ParallelLoop.h>

class HostSubroutine: public Subroutine
{

  protected:

    /*
     * ======================================================
     * Returns the name of the formal parameter which models the
     * name of the user subroutine
     * ======================================================
     */
    static std::string
    getUserSubroutineFormalParameterName ();

    /*
     * ======================================================
     * Returns the name of the formal parameter which models the
     * OP_SET
     * ======================================================
     */
    static std::string
    get_OP_SET_FormalParameterName ();

    /*
     * ======================================================
     * Returns the name of the indirection formal parameter
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_INDIRECTION_FormalParameterName (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Returns the name of the mapping formal parameter
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_MAP_FormalParameterName (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Returns the name of the access formal parameter
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_ACCESS_FormalParameterName (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Returns the name of the OP_DAT formal parameter
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_DAT_FormalParameterName (unsigned int OP_DAT_ArgumentGroup);

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
     * Returns the name of the OP_DAT device variable
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_DAT_DeviceVariableName (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Returns the name of the C to Fortran variable
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    getCToFortranVariableName (unsigned int OP_DAT_ArgumentGroup);

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
        SgScopeStatement * moduleScope);
};

#endif
