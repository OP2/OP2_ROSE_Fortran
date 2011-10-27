#pragma once
#ifndef FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_INDIRECT_LOOP_H
#define FORTRAN_CUDA_OPENMP_MODULE_DECLARATIONS_INDIRECT_LOOP_H

#include <FortranOpenMPModuleDeclarations.h>

class FortranOpenMPModuleDeclarationsIndirectLoop: public FortranOpenMPModuleDeclarations
{
  private:

    void
    createOpDatSizeDeclarations ();

    void
    createExecutionPlanDeclarations ();

  public:

    SgVarRefExp *
    getGlobalOpDatSizeDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVarRefExp *
    getGlobalToLocalMappingDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVarRefExp *
    getGlobalToLocalMappingSizeDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVarRefExp *
    getLocalToGlobalMappingDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVarRefExp *
    getLocalToGlobalMappingSizeDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVarRefExp *
    getPlanFunctionDeclaration (std::string const & variableName);

    FortranOpenMPModuleDeclarationsIndirectLoop (
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);

};

#endif
