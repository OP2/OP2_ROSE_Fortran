
#pragma once
#ifndef FORTRAN_OPENMP_KERNEL_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_OPENMP_KERNEL_SUBROUTINE_DIRECT_LOOP_H

#include <FortranOpenMPKernelSubroutine.h>

class FortranParallelLoop;

class FortranOpenMPKernelSubroutineDirectLoop: public FortranOpenMPKernelSubroutine
{
  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    virtual void
    createExecutionLoopStatements ();

    virtual void
    createOpDatFormalParameterDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranOpenMPKernelSubroutineDirectLoop (
        std::string const & subroutineName,
        std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
