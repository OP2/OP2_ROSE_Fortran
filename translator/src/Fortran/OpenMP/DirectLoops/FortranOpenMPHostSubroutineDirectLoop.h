#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <FortranOpenMPHostSubroutine.h>
#include <FortranOpenMPModuleDeclarationsDirectLoop.h>

class FortranOpenMPHostSubroutineDirectLoop: public FortranOpenMPHostSubroutine
{
  private:

    void
    createKernelDoLoop ();

    virtual void
    createTransferOpDatStatements (SgScopeStatement * statementScope);

    virtual void
    createFirstTimeExecutionStatements ();

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

    FortranOpenMPHostSubroutineDirectLoop (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope,
        FortranOpenMPModuleDeclarationsDirectLoop * moduleDeclarations);
};

#endif
