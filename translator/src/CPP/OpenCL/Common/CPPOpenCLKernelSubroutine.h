#ifndef CPP_OPENCL_KERNEL_SUBROUTINE_H
#define CPP_OPENCL_KERNEL_SUBROUTINE_H

#include <CPPKernelSubroutine.h>

class CPPOpenCLKernelSubroutine: public CPPKernelSubroutine
{
  protected:

    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration; //FIXME

  protected:
    void
    createReductionLoopStatements ();

    void
    createLocalThreadDeclarations ();

    void
    createAutoSharedDeclaration ();

    CPPOpenCLKernelSubroutine (
            std::string const & subroutineName,
            std::string const & userSubroutineName, 
            SgScopeStatement * moduleScope,
            CPPParallelLoop * parallelLoop,
            CPPReductionSubroutines * reductionSubroutines);
};

#endif
