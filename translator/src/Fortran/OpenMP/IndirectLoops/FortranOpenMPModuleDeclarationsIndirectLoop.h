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

    SgVariableDeclaration *
    getGlobalOpDatSizeDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVariableDeclaration *
    getGlobalToLocalMappingDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVariableDeclaration *
    getGlobalToLocalMappingSizeDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVariableDeclaration *
    getLocalToGlobalMappingDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVariableDeclaration *
    getLocalToGlobalMappingSizeDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SgVariableDeclaration *
    getPlanFunctionDeclaration (std::string const & variableName);

    SubroutineVariableDeclarations *
    getAllDeclarations ();

    FortranOpenMPModuleDeclarationsIndirectLoop (
        std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);

};

#endif
