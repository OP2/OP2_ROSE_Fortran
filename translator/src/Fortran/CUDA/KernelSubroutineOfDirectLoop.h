/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the kernel subroutine for a direct loop
 */

#ifndef KERNEL_SUBROUTINE_OF_DIRECT_LOOP_H
#define KERNEL_SUBROUTINE_OF_DIRECT_LOOP_H

#include <KernelSubroutine.h>
#include <DataSizesDeclarationOfDirectLoop.h>

class KernelSubroutineOfDirectLoop: public KernelSubroutine
{
  private:

    DataSizesDeclarationOfDirectLoop * dataSizesDeclarationOfDirectLoop;

  private:

    SgStatement *
    createUserSubroutineCall ();

    void
    create_OP_DAT_FormalParameters ();

    /*
     * ======================================================
     * Builds the statements included in the main
     * set elements-based loop
     * ======================================================
     */
    SgBasicBlock *
    buildMainLoopStatements ();

    /*
     * ======================================================
     * Builds the assignments of shared memory and local
     * thread variables, if needed
     * ======================================================
     */
    SgBasicBlock *
    stageInFromDeviceMemoryToLocalThreadVariables ();

    /*
     * ======================================================
     * Builds the assignments of shared memory and local
     * thread variables, if needed
     * ======================================================
     */
    SgBasicBlock *
    stageOutFromLocalThreadVariablesToDeviceMemory ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    KernelSubroutineOfDirectLoop (std::string const & subroutineName,
        UserDeviceSubroutine * userDeviceSubroutine,
        DataSizesDeclarationOfDirectLoop * dataSizesDeclarationOfDirectLoop,
        ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
