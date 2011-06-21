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
    getPlanFunctionVariableDeclaration (std::string const & variableName);

    FortranOpenMPModuleDeclarationsIndirectLoop (
        std::string const & userSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);

};

#endif
