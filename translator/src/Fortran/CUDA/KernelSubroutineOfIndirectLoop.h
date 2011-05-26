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
#include <DataSizesDeclarationOfIndirectLoop.h>

class KernelSubroutineOfIndirectLoop: public KernelSubroutine
{
  private:

    DataSizesDeclarationOfIndirectLoop * dataSizesDeclarationOfIndirectLoop;

    std::map <std::string, unsigned int> positionOf_nbytes;

    /*
     * ======================================================
     * Remaining local variables
     * ======================================================
     */

    std::map <unsigned int, SgVariableDeclaration *> localVariables_indArgSizes;

    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_incrementAccessMaps;

    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_incrementAccessArrays;

  private:

    static std::string
    getLocalToGlobalMappingVariableName (unsigned int OP_DAT_ArgumentGroup);

    static std::string
    getGlobalToLocalMappingVariableName (unsigned int OP_DAT_ArgumentGroup);

    static std::string
    getNumberOfBytesVariableName (unsigned int OP_DAT_ArgumentGroup);

    static std::string
    getRoundUpVariableName (unsigned int OP_DAT_ArgumentGroup);

    SgStatement *
    createUserSubroutineCall ();

    void
    applyPointeredIncrementsOrWrites ();

    void
    createPlanWhileLoop ();

    void
    createAutosharedWhileLoops ();

    void
    initialiseLocalVariables ();

    void
    createThreadZeroStatements ();

    void
    createRemainingLocalVariables ();

    void
    createLocalVariables ();

    void
    createSharedLocalVariables ();

    void
    createPlanFormalParameters ();

    void
    create_OP_DAT_FormalParameters ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

        KernelSubroutineOfIndirectLoop (
            std::string const & subroutineName,
            UserDeviceSubroutine * userDeviceSubroutine,
            DataSizesDeclarationOfIndirectLoop * dataSizesDeclarationOfIndirectLoop,
            ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);

};

#endif
