#pragma once
#ifndef CPP_OpenCL_HOST_SUBROUTINE_DIRECT_LOOP_H
#define CPP_OpenCL_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <CPPOpenCLHostSubroutine.h>

class CPPOpenCLDataSizesDeclarationDirectLoop;

class CPPOpenCLHostSubroutineDirectLoop: public CPPOpenCLHostSubroutine
{
  private:

    /*
     * ======================================================
     * Initialises OpenCL-specific variables
     * ======================================================
     */
    virtual void
    createOpenCLKernelInitialisationStatements ();

    /*
     * ======================================================
     * Creates the additional OpenCL-specific variables and
     * initialises them inside the host subroutine
     * ======================================================
     */
    void
    createOpenCLKernelLocalVariableDeclarationsForDirectLoop ();

    /*
     * ======================================================
     * Generates the code initialising the device variable
     * sizes variable
     * ======================================================
     */
    void
    createVariableSizesInitialisationStatements ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPOpenCLHostSubroutineDirectLoop (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        CPPOpenCLDataSizesDeclarationDirectLoop * dataSizesDeclaration,
        CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration);
};

#endif
