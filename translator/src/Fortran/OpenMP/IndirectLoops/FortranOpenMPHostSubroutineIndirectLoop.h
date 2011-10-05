
#pragma once
#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <FortranOpenMPHostSubroutine.h>
#include <FortranOpenMPModuleDeclarationsIndirectLoop.h>
#include <FortranPlan.h>

class FortranOpenMPHostSubroutineIndirectLoop: public FortranOpenMPHostSubroutine,
    public FortranPlan
{
  private:

    void
    createExecutionPlanDeclarations ();

    virtual SgBasicBlock *
    createTransferOpDatStatements ();

    virtual void
    createPlanFunctionExecutionStatements ();

    virtual void
    createFirstTimeExecutionStatements ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

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
        std::string const & kernelSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations);
};

#endif
