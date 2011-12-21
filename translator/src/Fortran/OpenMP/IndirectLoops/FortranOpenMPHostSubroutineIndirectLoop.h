#pragma once
#ifndef FORTRAN_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define FORTRAN_OPENMP_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <FortranOpenMPHostSubroutine.h>

class FortranOpenMPKernelSubroutine;
class FortranOpenMPModuleDeclarationsIndirectLoop;

class FortranOpenMPHostSubroutineIndirectLoop: public FortranOpenMPHostSubroutine
{
  private:

    SgBasicBlock *
    createConvertGlobalToLocalMappingStatements ();

    SgBasicBlock *
    createConvertLocalToGlobalMappingStatements ();

    SgBasicBlock *
    createPlanFunctionEpilogueStatements ();

    SgExprStatement *
    createPlanFunctionCallStatement ();

    SgBasicBlock *
    createSetUpOpDatTypeArrayStatements ();

    SgBasicBlock *
    createSetUpPlanFunctionActualParametersStatements ();

    SgBasicBlock *
    createPlanFunctionStatements ();

    void
    createPlanFunctionLocalVariableDeclarations ();

    virtual void
    createPlanFunctionExecutionStatements ();

    virtual void
    createKernelFunctionCallStatement (SgScopeStatement * scope);

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
