/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_CUDA_MODULE_DECLARATIONS_INDIRECT_LOOP_H
#define FORTRAN_CUDA_MODULE_DECLARATIONS_INDIRECT_LOOP_H

#include <FortranCUDAModuleDeclarations.h>
#include <FortranCUDADataSizesDeclarationIndirectLoop.h>

class FortranCUDAModuleDeclarationsIndirectLoop: public FortranCUDAModuleDeclarations
{
  private:

    void
    createCPlanDeclaration ();

  public:

    SgVariableDeclaration *
    getCPlanDeclaration ();

    FortranCUDAModuleDeclarationsIndirectLoop (
        std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclaration,
        FortranOpDatDimensionsDeclaration * dimensionsDeclaration);
};

#endif
