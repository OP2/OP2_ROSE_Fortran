#pragma once
#ifndef FORTRAN_OPENMP_KERNEL_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_OPENMP_KERNEL_SUBROUTINE_DIRECT_LOOP_H

#include <FortranOpenMPKernelSubroutine.h>

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

    FortranOpenMPKernelSubroutineDirectLoop (SgScopeStatement * moduleScope,
        Subroutine <SgProcedureHeaderStatement> * userSubroutine,
        FortranParallelLoop * parallelLoop);
  };

#endif
