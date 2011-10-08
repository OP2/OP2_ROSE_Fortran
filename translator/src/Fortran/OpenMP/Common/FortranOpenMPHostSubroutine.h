
#pragma once
#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>
#include <FortranParallelLoop.h>
#include <FortranOpenMPModuleDeclarations.h>

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
    createReductionLocalVariableDeclarations ();

    void
    initialiseNumberOfThreadsStatements ();

    FortranOpenMPHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranOpenMPModuleDeclarations * moduleDeclarations);
};

#endif
