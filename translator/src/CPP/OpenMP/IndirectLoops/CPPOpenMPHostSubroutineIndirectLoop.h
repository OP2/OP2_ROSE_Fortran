#pragma once
#ifndef CPP_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenMPHostSubroutine.h>

class CPPUserSubroutine;

class CPPOpenMPHostSubroutineIndirectLoop: public CPPOpenMPHostSubroutine
{
  private:

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    SgForStatement *
    createThreadLoopStatements ();

    SgOmpParallelStatement *
    createOpenMPLoopStatements ();

    SgBasicBlock *
    createInitialisePlanFunctionVariablesStatements ();

    virtual void
    createStatements ();

    void
    createIncrementAccessLocalVariableDeclarations ();

    void
    createPlanFunctionDeclarations ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPOpenMPHostSubroutineIndirectLoop (SgScopeStatement * moduleScope,
        CPPUserSubroutine * calleeSubroutine, CPPParallelLoop * parallelLoop);
};

#endif
