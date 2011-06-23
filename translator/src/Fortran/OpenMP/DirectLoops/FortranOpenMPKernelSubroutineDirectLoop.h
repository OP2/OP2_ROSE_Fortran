#ifndef FORTRAN_OPENMP_KERNEL_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_OPENMP_KERNEL_SUBROUTINE_DIRECT_LOOP_H

#include <FortranOpenMPKernelSubroutine.h>
#include <ParallelLoop.h>

class FortranOpenMPKernelSubroutineDirectLoop: public FortranOpenMPKernelSubroutine
{
  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    void
    createUserSubroutineDoLoop ();

    void
    initialiseThreadID ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranOpenMPKernelSubroutineDirectLoop (
        std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif
