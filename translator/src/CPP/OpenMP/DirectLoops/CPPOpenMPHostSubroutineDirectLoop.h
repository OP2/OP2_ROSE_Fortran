#pragma once
#ifndef CPP_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H
#define CPP_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <CPPOpenMPHostSubroutine.h>

class CPPUserSubroutine;

class CPPOpenMPHostSubroutineDirectLoop: public CPPOpenMPHostSubroutine
{
  private:

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    SgForStatement *
    createThreadLoopStatements ();

    SgOmpParallelStatement *
    createOpenMPLoopStatements ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPOpenMPHostSubroutineDirectLoop (SgScopeStatement * moduleScope,
        CPPUserSubroutine * calleeSubroutine, CPPParallelLoop * parallelLoop);
};

#endif
