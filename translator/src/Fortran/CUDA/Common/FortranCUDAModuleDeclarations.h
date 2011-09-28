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
    createReductionDeclarations ();

    void
    createDataSizesDeclaration ();

    void
    createDimensionsDeclaration ();

  public:

    SgVariableDeclaration *
    getDataSizesVariableDeclaration ();

    SgVariableDeclaration *
    getDimensionsVariableDeclaration ();

    FortranCUDAModuleDeclarations (FortranParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope,
        FortranCUDADataSizesDeclaration * dataSizesDeclaration,
        FortranOpDatDimensionsDeclaration * dimensionsDeclaration);
};

#endif
