#pragma once
#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <FortranOpenMPHostSubroutine.h>

class FortranOpenMPKernelSubroutine;
class FortranOpenMPModuleDeclarationsDirectLoop;

class FortranOpenMPHostSubroutineDirectLoop: public FortranOpenMPHostSubroutine
{
  private:

    void
    createKernelDoLoop ();

    virtual SgBasicBlock *
    createTransferOpDatStatements ();

    void
    initialiseThreadVariablesStatements ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    virtual void
    createOpenMPVariableDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    FortranOpenMPHostSubroutineDirectLoop (SgScopeStatement * moduleScope,
        FortranOpenMPKernelSubroutine * kernelSubroutine,
        FortranParallelLoop * parallelLoop,
        FortranOpenMPModuleDeclarationsDirectLoop * moduleDeclarations);
};

#endif
