/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the host subroutine for an indirect loop.
 * Its declarations and statements do the following:
 */

#ifndef CPP_OPENCL_HOST_SUBROUTINE_INDIRECT_LOOP_H
#define CPP_OPENCL_HOST_SUBROUTINE_INDIRECT_LOOP_H

#include <CPPOpenCLHostSubroutine.h>
#include <CPPOpenCLKernelSubroutine.h>
#include <CPPOpenCLModuleDeclarationsIndirectLoop.h>
#include <CPPOpenCLDataSizesDeclarationIndirectLoop.h>
#include <CPPPlan.h>

class CPPOpenCLHostSubroutineIndirectLoop: public CPPOpenCLHostSubroutine,
    public CPPPlan
{
  private:

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    void
    createPlanFunctionExecutionStatements ();

    void
    createVariablesSizesInitialisationStatements ();

    void
    createExecutionPlanDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

  public:

        CPPOpenCLHostSubroutineIndirectLoop (
            std::string const & subroutineName,
            std::string const & userSubroutineName,
            std::string const & kernelSubroutineName,
            CPPParallelLoop * parallelLoop,
            SgScopeStatement * moduleScope,
            CPPInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
            CPPOpenCLDataSizesDeclarationIndirectLoop * dataSizesDeclaration,
            CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
            CPPOpenCLModuleDeclarationsIndirectLoop * moduleDeclarations);
};

#endif
