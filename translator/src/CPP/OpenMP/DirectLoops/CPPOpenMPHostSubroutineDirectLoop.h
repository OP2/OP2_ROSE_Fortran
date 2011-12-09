#pragma once
#ifndef CPP_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H
#define CPP_OPENMP_HOST_SUBROUTINE_DIRECT_LOOP_H

#include <CPPOpenMPHostSubroutine.h>

class CPPOpenMPHostSubroutineDirectLoop: public CPPOpenMPHostSubroutine
{
  private:

    void
    createOpenMPLocalVariableDeclarations ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    void
    createOpenMPLoopStatements ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPOpenMPHostSubroutineDirectLoop (SgScopeStatement * moduleScope,
        CPPOpenMPKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPModuleDeclarations * moduleDeclarations);
};

#endif
