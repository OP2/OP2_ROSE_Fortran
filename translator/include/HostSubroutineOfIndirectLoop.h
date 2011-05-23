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

    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_ExecutionPlan_OP_DAT;

    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_ExecutionPlan_OP_DAT_Size;

    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_ExecutionPlan_OP_MAP;

    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_ExecutionPlan_OP_MAP_Size;

    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_ExecutionPlan_IndirectMaps;

    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_ExecutionPlan_IndirectMaps_Size;

  private:

    void
        initialiseDeviceVariablesSizesVariable (
            ParallelLoop & parallelLoop,
            DataSizesDeclarationOfIndirectLoop & dataSizesDeclarationOfIndirectLoop);

    void
    createExecutionPlanExecutionStatements (
        KernelSubroutine & kernelSubroutine, ParallelLoop & parallelLoop);

    void
    initialiseVariablesAndConstants (
        InitialiseConstantsSubroutine & initialiseConstantsSubroutine);

    void
    createPlanCToForttranPointerConversionStatements (
        ParallelLoop & parallelLoop);

    void
    createPlanFunctionCallStatement ();

    void
    createDoLoopToCorrectIndexing (ParallelLoop & parallelLoop);

    void
    createExecutionPlanStatements (ParallelLoop & parallelLoop);

    void
        createExecutionPlanLocalVariables (
            DataSizesDeclarationOfIndirectLoop & dataSizesDeclarationOfIndirectLoop,
            ParallelLoop & parallelLoop);

  public:

        HostSubroutineOfIndirectLoop (
            std::string const & subroutineName,
            UserDeviceSubroutine & userDeviceSubroutine,
            KernelSubroutine & kernelSubroutine,
            InitialiseConstantsSubroutine & initialiseConstantsSubroutine,
            DataSizesDeclarationOfIndirectLoop & dataSizesDeclarationOfIndirectLoop,
            ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);
};

#endif
