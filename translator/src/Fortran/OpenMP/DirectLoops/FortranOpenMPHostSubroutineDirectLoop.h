#pragma once
#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <FortranOpenMPHostSubroutine.h>

class FortranOpenMPKernelSubroutine;

class FortranOpenMPHostSubroutineDirectLoop: public FortranOpenMPHostSubroutine
{
  private:

    void
    createOpenMPLoopStatements ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    FortranOpenMPHostSubroutineDirectLoop (SgScopeStatement * moduleScope,
        FortranOpenMPKernelSubroutine * kernelSubroutine,
        FortranParallelLoop * parallelLoop);
};

#endif
