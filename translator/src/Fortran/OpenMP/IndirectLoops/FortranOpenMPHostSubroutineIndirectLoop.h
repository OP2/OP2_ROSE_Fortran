#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <FortranOpenMPHostSubroutine.h>
#include <FortranOpenMPModuleDeclarationsIndirectLoop.h>

class FortranOpenMPHostSubroutineIndirectLoop: public FortranOpenMPHostSubroutine
{
  private:

    FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations;

  private:

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    FortranOpenMPHostSubroutineIndirectLoop (
        std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope,
        FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations);
};

#endif
