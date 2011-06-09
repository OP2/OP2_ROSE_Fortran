#ifndef FORTRAN_OPENMP_KERNEL_SUBROUTINE_INDIRECT_LOOP_H
#define FORTRAN_OPENMP_KERNEL_SUBROUTINE_INDIRECT_LOOP_H

#include <FortranOpenMPKernelSubroutine.h>
#include <ParallelLoop.h>

class FortranOpenMPKernelSubroutineIndirectLoop: public FortranOpenMPKernelSubroutine
{
  private:

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranOpenMPKernelSubroutineIndirectLoop (
        std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
