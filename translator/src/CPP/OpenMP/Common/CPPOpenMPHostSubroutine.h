#pragma once
#ifndef CPP_OPENMP_HOST_SUBROUTINE_H
#define CPP_OPENMP_HOST_SUBROUTINE_H

#include <CPPHostSubroutine.h>

class CPPOpenMPKernelSubroutine;
class CPPModuleDeclarations;

class CPPOpenMPHostSubroutine: public CPPHostSubroutine
{
  protected:

    CPPModuleDeclarations * moduleDeclarations;

  protected:

    SgBasicBlock *
    createInitialiseNumberOfThreadsStatements ();

    SgBasicBlock *
    createOpDatTypeCastStatements ();

    void
    createOpDatTypeCastVariableDeclarations ();

    virtual void
    createReductionEpilogueStatements ();

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionDeclarations ();

    CPPOpenMPHostSubroutine (SgScopeStatement * moduleScope,
        CPPOpenMPKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPModuleDeclarations * moduleDeclarations);
};

#endif
