#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <FortranOpenMPHostSubroutine.h>
#include <FortranOpenMPModuleDeclarations.h>

class FortranOpenMPHostSubroutineDirectLoop: public FortranOpenMPHostSubroutine
{
  private:

    SgStatement *
    createKernelCall ();

    void
    createKernelDoLoop ();

    void
    createCToFortranPointerConversionStatements ();

    void
    initialiseThreadVariables ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    FortranOpenMPHostSubroutineDirectLoop (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope,
        FortranOpenMPModuleDeclarations * moduleDeclarations);
};

#endif
