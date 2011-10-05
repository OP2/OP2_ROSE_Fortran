/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the host subroutine for a direct loop
 */

#pragma once
#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <FortranCUDAHostSubroutine.h>

class FortranCUDAModuleDeclarations;

class FortranCUDAHostSubroutineDirectLoop: public FortranCUDAHostSubroutine
{
  private:

    /*
     * ======================================================
     * Initialises CUDA-specific variables
     * ======================================================
     */
    void
    createCUDAKernelInitialisationStatements ();

    /*
     * ======================================================
     * Creates the additional CUDA-specific variables and
     * initialises them inside the host subroutine
     * ======================================================
     */
    void
    createCUDAKernelLocalVariableDeclarationsForDirectLoop ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    FortranCUDAHostSubroutineDirectLoop (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
        FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
