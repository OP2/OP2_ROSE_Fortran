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
    createOpenMPVariableDeclarations () = 0;

    virtual void
    createReductionEpilogueStatements ();

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionDeclarations ();

    void
    initialiseNumberOfThreadsStatements ();

    FortranOpenMPHostSubroutine (SgScopeStatement * moduleScope,
        FortranKernelSubroutine * kernelSubroutine,
        FortranParallelLoop * parallelLoop,
        FortranOpenMPModuleDeclarations * moduleDeclarations);
};

#endif
