#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>
#include <ParallelLoop.h>
#include <FortranOpenMPModuleDeclarations.h>

class FortranOpenMPHostSubroutine: public FortranHostSubroutine
{
  protected:

    FortranOpenMPModuleDeclarations * moduleDeclarations;

  protected:

    virtual void
    createFirstTimeExecutionStatements () = 0;

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
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope,
        FortranOpenMPModuleDeclarations * moduleDeclarations);
};

#endif
