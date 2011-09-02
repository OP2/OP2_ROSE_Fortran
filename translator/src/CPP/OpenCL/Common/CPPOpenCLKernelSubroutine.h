#ifndef CPP_OPENCL_KERNEL_SUBROUTINE_H
#define CPP_OPENCL_KERNEL_SUBROUTINE_H

#include <CPPKernelSubroutine.h>
#include <CPPOpDatDimensionsDeclaration.h>

class CPPOpenCLKernelSubroutine: public CPPKernelSubroutine
{
  protected:

    CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration; //FIXME

  protected:
    void
    createReductionLoopStatements ();

    void
    createLocalThreadDeclarations ();

    void
    createAutoSharedDeclarations ();

    CPPOpenCLKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, CPPParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope,
        CPPReductionSubroutines * reductionSubroutines,
        CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration);
};

#endif
