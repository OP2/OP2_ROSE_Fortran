/*
 * Written by Adam Betts and Carlo Bertolli
 */

#pragma once
#ifndef FORTRAN_CUDA_MODULE_DECLARATIONS_H
#define FORTRAN_CUDA_MODULE_DECLARATIONS_H

#include <FortranModuleDeclarations.h>

class FortranCUDAOpDatCardinalitiesDeclaration;
class FortranOpDatDimensionsDeclaration;

class FortranCUDAModuleDeclarations: public FortranModuleDeclarations
{
  protected:

    FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration;

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
        FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
        FortranOpDatDimensionsDeclaration * dimensionsDeclaration);
};

#endif
