/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the kernel subroutine for a direct loop
 *
 */

#ifndef KERNEL_SUBROUTINE_OF_DIRECT_LOOP_H
#define KERNEL_SUBROUTINE_OF_DIRECT_LOOP_H

#include <KernelSubroutine.h>
#include <DeviceDataSizesDeclarationDirectLoops.h>

class KernelSubroutineOfDirectLoop: public KernelSubroutine
{
  private:

    /*
     * ======================================================
     * Creates the statements inside the kernel subroutine
     * ======================================================
     */
    void
    createStatements (UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Creates local variables inside the kernel subroutine
     * ======================================================
     */
    void
    createLocalVariables (ParallelLoop & parallelLoop);

    void
    create_OP_DAT_FormalParameters (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Builds the statements included in the main
     * set elements-based loop
     * ======================================================
     */
    SgBasicBlock *
    buildMainLoopStatements (UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Builds the assignments of shared memory and local
     * thread variables, if needed
     * ======================================================
     */
    SgBasicBlock *
    stageInFromDeviceMemoryToLocalThreadVariables (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Builds the assignments of shared memory and local
     * thread variables, if needed
     * ======================================================
     */
    SgBasicBlock
        *
        stageOutFromLocalThreadVariablesToDeviceMemory (
            ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Creates all formal parameters
     * ======================================================
     */

    void
        createAndAppendFormalParameters (
            DeviceDataSizesDeclarationDirectLoops & DeviceDataSizesDeclarationDirectLoops,
            ParallelLoop & parallelLoop);

  public:

        KernelSubroutineOfDirectLoop (
            std::string const & subroutineName,
            UserDeviceSubroutine & userDeviceSubroutine,
            DeviceDataSizesDeclarationDirectLoops & DeviceDataSizesDeclarationDirectLoops,
            std::map <unsigned int, SgProcedureHeaderStatement *> & reductSubroutines,
            ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);
};

#endif
