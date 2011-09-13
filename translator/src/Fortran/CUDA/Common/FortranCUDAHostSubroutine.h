#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>
#include <FortranCUDAKernelSubroutine.h>
#include <FortranCUDADataSizesDeclaration.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranParallelLoop.h>
#include <FortranCUDAModuleDeclarations.h>

class FortranCUDAHostSubroutine: public FortranHostSubroutine
{
  protected:

    FortranCUDADataSizesDeclaration * dataSizesDeclaration;

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

    /*
     * ======================================================
     * Creates the statements which are executed when the
     * host subroutine is executed for the first time
     * ======================================================
     */
    SgBasicBlock *
    createFirstTimeExecutionStatements ();

    void
    createCUDAKernelLocalVariableDeclarations ();

    void
    createDataMarshallingLocalVariableDeclarations ();

    FortranCUDAHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranCUDADataSizesDeclaration * dataSizesDeclaration,
        FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
