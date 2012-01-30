#pragma once
#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>

class FortranKernelSubroutine;
class FortranCUDAOpDatCardinalitiesDeclaration;
class FortranOpDatDimensionsDeclaration;
class FortranCUDAModuleDeclarations;

class FortranCUDAHostSubroutine: public FortranHostSubroutine
{
  protected:

    FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration;

    FortranOpDatDimensionsDeclaration * dimensionsDeclaration;

    FortranCUDAModuleDeclarations * moduleDeclarations;

  protected:

    virtual void
    createReductionEpilogueStatements ();

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionDeclarations ();

    SgExpression *
    getOpDatCardinalityInitialisationExpression (SgScopeStatement * scope,
        unsigned int OP_DAT_ArgumentGroup);

    virtual SgBasicBlock *
    createTransferOpDatStatements ();

    SgBasicBlock *
    createDeallocateStatements ();

    void
    createCUDAConfigurationLaunchDeclarations ();

    void
    createDataMarshallingDeclarations ();

    void
    createOpDatCardinalitiesDeclaration ();

    void
    createOpDatDimensionsDeclaration ();

    void
    createIterationVariablesDeclarations ();
    
    FortranCUDAHostSubroutine (SgScopeStatement * moduleScope,
        FortranKernelSubroutine * kernelSubroutine,
        FortranParallelLoop * parallelLoop,
        FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration,
        FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
