#pragma once
#ifndef CPP_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenMPHostSubroutine.h>

class CPPOpenMPHostSubroutineIndirectLoop: public CPPOpenMPHostSubroutine
{
  private:

    void
    createOpenMPLoopStatements (SgScopeStatement * scope);

    SgExprStatement *
    createPlanFunctionCallStatement ();

    SgBasicBlock *
    createPlanFunctionExecutionStatements ();

    void
    createPlanFunctionDeclarations ();

    void
    createOpenMPLocalVariableDeclarations ();

    virtual void
    createKernelFunctionCallStatement (SgScopeStatement * scope);

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPOpenMPHostSubroutineIndirectLoop (SgScopeStatement * moduleScope,
        CPPOpenMPKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPModuleDeclarations * moduleDeclarations);
};

#endif
