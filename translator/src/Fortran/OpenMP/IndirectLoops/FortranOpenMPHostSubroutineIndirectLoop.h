#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <FortranOpenMPHostSubroutine.h>
#include <FortranOpenMPModuleDeclarationsIndirectLoop.h>
#include <FortranPlan.h>

class FortranOpenMPHostSubroutineIndirectLoop: public FortranOpenMPHostSubroutine
{
  private:

    FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations;

  private:

    virtual void
    createFirstTimeExecutionStatements ();

    void
    createExecutionPlanDeclarations ();

    virtual SgStatement *
    createStatementToCallKernelFunction ();

    virtual void
    createStatementsToConvertCPointers (
        std::vector <SgStatement *> & statements);

    virtual void
    createOpenMPVariableDeclarations ();

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
