#pragma once
#ifndef CPP_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenMPHostSubroutine.h>

class CPPOpenMPHostSubroutineIndirectLoop: public CPPOpenMPHostSubroutine
{
  private:

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    void
    createOpenMPLoopStatements (SgScopeStatement * scope);

    SgBasicBlock *
    createPlanFunctionExecutionStatements ();

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
        CPPOpenMPKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop);
};

#endif
