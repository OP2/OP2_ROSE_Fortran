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

/*
 * ======================================================
 * These are names of local variables needed in the host
 * subroutine of an indirect loop. They are contained in
 * an anonymous namespace so that they have internal linkage,
 * i.e. they cannot be accessed outside of the scope of
 * this header file
 * ======================================================
 */
namespace
{
  std::string const variableName_IterationCounter = "i";
  std::string const variableName_argsNumber = "argsNumber";
  std::string const variableName_indsNumber = "indsNumber";
  std::string const variableName_planRet = "planRet";
  std::string const variableName_actualPlan = "actualPlan";
  std::string const variableName_args = "args";
  std::string const variableName_idxs = "idxs";
  std::string const variableName_maps = "maps";
  std::string const variableName_accesses = "accesses";
  std::string const variableName_inds = "inds";
  std::string const variableName_blockOffset = "blockOffset";
  std::string const variableName_col = "col";
  std::string const variableName_threadSynchRet = "threadSynchRet";
  std::string const variableName_pindSizesSize = "pindSizesSize";
  std::string const variableName_pindOffsSize = "pindOffsSize";
  std::string const variableName_pblkMapSize = "pblkMapSize";
  std::string const variableName_poffsetSize = "poffsetSize";
  std::string const variableName_pnelemsSize = "pnelemsSize";
  std::string const variableName_pnthrcolSize = "pnthrcolSize";
  std::string const variableName_pthrcolSize = "pthrcolSize";
  std::string const variableName_ncolblk = "ncolblk";
  std::string const variableName_pnindirect = "pnindirect";
  std::string const variableName_pindSizes = "pindSizes";
  std::string const variableName_pindOffs = "pindOffs";
  std::string const variableName_pblkMap = "pblkMap";
  std::string const variableName_poffset = "poffset";
  std::string const variableName_pnelems = "pnelems";
  std::string const variableName_pnthrcol = "pnthrcol";
  std::string const variableName_pthrcol = "pthrcol";
  std::string const variableName_pindMaps = "pindMaps";
  std::string const variableName_pindMapsSize = "pindMapsSize";
  std::string const variableName_pmaps = "pmaps";
}

class HostSubroutineOfIndirectLoop: public HostSubroutine
{
  private:

    std::map <std::string, SgVariableDeclaration *> localVariables_Others;

    std::map <std::string, SgVariableDeclaration *>
        localVariables_ExecutionPlan_OP_DAT;

    std::map <std::string, SgVariableDeclaration *>
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
    createPlanCToForttranPointerConversionStatements (
        ParallelLoop & parallelLoop);

    void
    createPlanFunctionCallStatement ();

    void
    createDoLoopToCorrectIndexing (ParallelLoop & parallelLoop);

    void
    createExecutionPlanStatements (ParallelLoop & parallelLoop);

    void
    createExecutionPlanLocalVariables (ParallelLoop & parallelLoop);

  public:

    HostSubroutineOfIndirectLoop (std::string const & subroutineName,
        UserDeviceSubroutine & userDeviceSubroutine,
        KernelSubroutine & kernelSubroutine, ParallelLoop & parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
