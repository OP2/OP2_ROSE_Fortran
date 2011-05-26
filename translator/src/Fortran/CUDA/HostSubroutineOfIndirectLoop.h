/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the host subroutine for an indirect loop.
 * Its declarations and statements do the following:
 */

#ifndef HOST_SUBROUTINE_OF_INDIRECT_LOOP_H
#define HOST_SUBROUTINE_OF_INDIRECT_LOOP_H

#include <HostSubroutine.h>
#include <KernelSubroutine.h>
#include <InitialiseConstantsSubroutine.h>
#include <DataSizesDeclarationOfIndirectLoop.h>

class HostSubroutineOfIndirectLoop: public HostSubroutine
{
  private:

    InitialiseConstantsSubroutine * initialiseConstantsSubroutine;

    DataSizesDeclarationOfIndirectLoop * dataSizesDeclarationOfIndirectLoop;

  private:

    /*
     * ======================================================
     * Returns the name of the local variable which models
     * the mappings from local indices to global indices in
     * shared memory
     * ======================================================
     */
    static std::string
    getLocalToGlobalMappingVariableName (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Returns the name of the local variable which models
     * the indirect mappings to local indices in shared memory
     * ======================================================
     */
    static std::string
    getGlobalToLocalMappingVariableName (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Returns the name of the local variable which models
     * the number of indirect elements in each block
     * ======================================================
     */
    static std::string
    getGlobalToLocalMappingSizeVariableName (unsigned int OP_DAT_ArgumentGroup);

    void
    initialiseDeviceVariablesSizesVariable ();

    void
    createExecutionPlanExecutionStatements ();

    void
    initialiseVariablesAndConstants ();

    void
    createPlanCToForttranPointerConversionStatements ();

    void
    createPlanFunctionCallStatement ();

    void
    createDoLoopToCorrectIndexing ();

    void
    createExecutionPlanStatements ();

    void
    createExecutionPlanLocalVariables ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

        HostSubroutineOfIndirectLoop (
            std::string const & subroutineName,
            UserDeviceSubroutine * userDeviceSubroutine,
            KernelSubroutine * kernelSubroutine,
            InitialiseConstantsSubroutine * initialiseConstantsSubroutine,
            DataSizesDeclarationOfIndirectLoop * dataSizesDeclarationOfIndirectLoop,
            ParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
