#pragma once
#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>

class FortranCUDAOpDatCardinalitiesDeclaration;
class FortranOpDatDimensionsDeclaration;
class FortranCUDAModuleDeclarations;

class FortranCUDAHostSubroutine: public FortranHostSubroutine
{
  protected:

    FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration;

    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration;

    FortranCUDAModuleDeclarations * moduleDeclarations;

  protected:

    virtual void
    createReductionEpilogueStatements ();

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionLocalVariableDeclarations ();

    /*
     * ======================================================
     * Creates the expression on the right-hand side of the
     * statement which initialises the size of the OP_DAT in this
     * argument group
     * ======================================================
     */
    SgExpression *
    createRHSOfInitialiseOpDatSizeStatement (SgScopeStatement * scope,
        unsigned int OP_DAT_ArgumentGroup);

    virtual SgBasicBlock *
    createTransferOpDatStatements ();

    void
    createCUDAKernelLocalVariableDeclarations ();

    void
    createDataMarshallingLocalVariableDeclarations ();

    void
    createOpDatCardinalitiesDeclaration ();

    void
    createOpDatDimensionsDeclaration ();

    FortranCUDAHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
        FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
