#pragma once
#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <FortranOpenMPHostSubroutine.h>

class FortranOpenMPKernelSubroutine;
class FortranOpenMPModuleDeclarationsIndirectLoop;

class FortranOpenMPHostSubroutineIndirectLoop: public FortranOpenMPHostSubroutine
{

  private:

    void
    createExecutionPlanDeclarations ();

    virtual SgBasicBlock *
    createTransferOpDatStatements ();

    virtual void
    createPlanFunctionExecutionStatements ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    virtual void
    createOpenMPVariableDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    FortranOpenMPHostSubroutineIndirectLoop (SgScopeStatement * moduleScope,
        FortranOpenMPKernelSubroutine * kernelSubroutine,
        FortranParallelLoop * parallelLoop,
        FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations);
};

#endif
