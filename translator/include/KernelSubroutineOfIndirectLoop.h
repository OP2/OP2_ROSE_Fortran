/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the kernel subroutine for an indirect loop.
 * Its declarations and statements do the following:
 *
 */

#ifndef KERNEL_SUBROUTINE_OF_INDIRECT_LOOP_H
#define KERNEL_SUBROUTINE_OF_INDIRECT_LOOP_H

#include <KernelSubroutine.h>
#include <DeviceDataSizesDeclaration.h>

class KernelSubroutineOfIndirectLoop: public KernelSubroutine
{
  private:

    std::map <std::string, SgVariableDeclaration *>
        formalParameters_PlanVariables;

    std::map <unsigned int, SgVariableDeclaration *>
        formalParameters_LocalToGlobalMapping;

    std::map <unsigned int, SgVariableDeclaration *>
        formalParameters_GlobalToLocalMapping;

    /*
     * ======================================================
     * Remaining local variables
     * ======================================================
     */

    std::map <unsigned int, SgVariableDeclaration *> localVariables_indArgSizes;

    std::map <std::string, SgVariableDeclaration *> localVariables_Others;

  private:

    void
    createThreadZeroStatements (ParallelLoop & parallelLoop);

    void
    createRemainingLocalVariables ();

    void
    createSharedLocalVariables (ParallelLoop & parallelLoop);

    void
    createPlanFormalParameters (
        DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
        ParallelLoop & parallelLoop);

    void
    create_OP_DAT_FormalParameters (
        DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
        ParallelLoop & parallelLoop);

  public:

    KernelSubroutineOfIndirectLoop (std::string const & subroutineName,
        UserDeviceSubroutine & userDeviceSubroutine,
        DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
        ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);

};

#endif
