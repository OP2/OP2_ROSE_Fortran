#pragma once
#ifndef CPP_OPENCL_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENCL_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenCLHostSubroutine.h>
#include <CPPPlan.h>

class CPPOpenCLDataSizesDeclarationIndirectLoop;

class CPPOpenCLHostSubroutineIndirectLoop: public CPPOpenCLHostSubroutine,
    public CPPPlan
{
  private:
    void
    createPlanFunctionExecutionStatements ();

    void
    createVariablesSizesInitialisationStatements ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    void
    createExecutionPlanDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

    CPPOpenCLHostSubroutineIndirectLoop (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        //            CPPInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
        CPPOpenCLDataSizesDeclarationIndirectLoop * dataSizesDeclaration,
        CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration);
};

#endif
