/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_CUDA_MODULE_DECLARATIONS_H
#define FORTRAN_CUDA_MODULE_DECLARATIONS_H

#include <FortranModuleDeclarations.h>
#include <FortranCUDADataSizesDeclaration.h>
#include <FortranOpDatDimensionsDeclaration.h>

class FortranCUDAModuleDeclarations: public FortranModuleDeclarations
{
  protected:

    FortranCUDADataSizesDeclaration * dataSizesDeclaration;

    FortranOpDatDimensionsDeclaration * dimensionsDeclaration;

  protected:

    void
    createDataSizesDeclaration ();

    void
    createDimensionsDeclaration ();

    FortranCUDAModuleDeclarations (std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranCUDADataSizesDeclaration * dataSizesDeclaration,
        FortranOpDatDimensionsDeclaration * dimensionsDeclaration);

  public:

    SgVariableDeclaration *
    getDataSizesVariableDeclaration ();

    FortranCUDADataSizesDeclaration *
    getDataSizesTypeDeclaration ();

    SgVariableDeclaration *
    getDimensionsVariableDeclaration ();

    FortranOpDatDimensionsDeclaration *
    getDimensionsTypeDeclaration ();
};

#endif
