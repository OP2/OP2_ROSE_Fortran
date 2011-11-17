#pragma once
#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>

class FortranParallelLoop;
class FortranKernelSubroutine;
class FortranOpenMPModuleDeclarations;

class FortranOpenMPHostSubroutine: public FortranHostSubroutine
{
  protected:

    FortranOpenMPModuleDeclarations * moduleDeclarations;

  protected:

    virtual void
    createReductionEpilogueStatements ();

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionDeclarations ();

    SgBasicBlock *
    createInitialiseNumberOfThreadsStatements ();

    virtual SgBasicBlock *
    createTransferOpDatStatements ();

    void
    createOpDatLocalVariableDeclarations ();

    void
    createOpenMPLocalVariableDeclarations ();

    FortranOpenMPHostSubroutine (SgScopeStatement * moduleScope,
        FortranKernelSubroutine * kernelSubroutine,
        FortranParallelLoop * parallelLoop);
};

#endif
