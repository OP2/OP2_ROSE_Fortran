#pragma once
#ifndef CPP_OPENCL_KERNEL_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENCL_KERNEL_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenCLKernelSubroutine.h>
#include <CPPOpenCLDataSizesDeclarationIndirectLoop.h>

class CPPOpenCLKernelSubroutineIndirectLoop: public CPPOpenCLKernelSubroutine
{
  private:

    CPPOpenCLDataSizesDeclarationIndirectLoop * dataSizesDeclaration;

    std::map <std::string, unsigned int> positionOfNbytes;

  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    virtual void
    createExecutionLoopStatements ();

    void
    createInnerExecutionLoopStatements (SgScopeStatement * scope);

    void
    createIncrementAdjustmentStatements (SgScopeStatement * scope);

    void
    createInitialiseLocalOpDatStatements (SgScopeStatement * scope);

    void
    createPointeredIncrementsOrWritesStatements ();

    void
    createAutoSharedWhileLoopStatements ();

    void
    createInitialiseLocalVariablesStatements ();

    void
    createThreadZeroStatements ();

    void
    createPlanFormalParameterDeclarations ();

    virtual void
    createOpDatFormalParameterDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPOpenCLKernelSubroutineIndirectLoop (std::string const & subroutineName,
        std::string const & userSubroutineName, CPPParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope,
        CPPReductionSubroutines * reductionSubroutines,
        CPPOpenCLDataSizesDeclarationIndirectLoop * dataSizesDeclaration,
        CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration);

};

#endif
