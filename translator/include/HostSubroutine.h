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

    /*
     * ======================================================
     * Returns the name of the formal parameter which models the
     * name of the user subroutine
     * ======================================================
     */
    static std::string
    getUserSubroutineFormalParameterName ()
    {
      return "subroutineName";
    }

    /*
     * ======================================================
     * Returns the name of the formal parameter which models the
     * OP_SET
     * ======================================================
     */
    static std::string
    get_OP_SET_FormalParameterName ()
    {
      return "set";
    }

    /*
     * ======================================================
     * Returns the name of the indirection formal parameter
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_INDIRECTION_FormalParameterName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_INDIRECTION
          + lexical_cast <string> (OP_DAT_ArgumentGroup);
    }

    /*
     * ======================================================
     * Returns the name of the mapping formal parameter
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_MAP_FormalParameterName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_MAP
          + lexical_cast <string> (OP_DAT_ArgumentGroup);
    }

    /*
     * ======================================================
     * Returns the name of the access formal parameter
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_ACCESS_FormalParameterName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_ACCESS
          + lexical_cast <string> (OP_DAT_ArgumentGroup);
    }

    /*
     * ======================================================
     * Returns the name of the OP_DAT formal parameter
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_DAT_FormalParameterName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
          + lexical_cast <string> (OP_DAT_ArgumentGroup);
    }

    /*
     * ======================================================
     * Returns the name of the variable modelling the size of
     * an OP_DAT in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_DAT_SizeVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
          + lexical_cast <string> (OP_DAT_ArgumentGroup)
          + IndirectAndDirectLoop::Fortran::VariableSuffixes::Size;
    }

    /*
     * ======================================================
     * Returns the name of the OP_DAT device variable
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_DAT_DeviceVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
          + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Device";
    }

    /*
     * ======================================================
     * Returns the name of the C to Fortran variable
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    getCToFortranVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return "cFortranPointer" + lexical_cast <string> (OP_DAT_ArgumentGroup);
    }

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
};

#endif
