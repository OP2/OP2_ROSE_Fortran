/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_CUDA_MODULE_DECLARATIONS_H
#define FORTRAN_CUDA_MODULE_DECLARATIONS_H

#include <FortranModuleDeclarations.h>
#include <FortranCUDADataSizesDeclaration.h>

class FortranCUDAModuleDeclarations: public FortranModuleDeclarations
{
  protected:

    FortranCUDADataSizesDeclaration * dataSizesDeclaration;

  protected:

    void
    createDataSizesDeclaration ();

    FortranCUDAModuleDeclarations (std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranCUDADataSizesDeclaration * dataSizesDeclaration);

  public:

    SgVariableDeclaration *
    getDataSizesVariableDeclaration ();

    FortranCUDADataSizesDeclaration *
    getDataSizesDeclaration ();
};

#endif
