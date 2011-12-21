#pragma once
#ifndef CPP_OPENCL_HOST_SUBROUTINE_H
#define CPP_OPENCL_HOST_SUBROUTINE_H

#include <CPPHostSubroutine.h>

class CPPOpenCLKernelSubroutine;
class CPPModuleDeclarations;
class CPPUserSubroutine;
class CPPOpenCLConstantDeclarations;
class SgScopeStatement;

class CPPOpenCLHostSubroutine: public CPPHostSubroutine
{
  protected:

    CPPModuleDeclarations * moduleDeclarations;

    CPPUserSubroutine * userSubroutine;

    CPPOpenCLConstantDeclarations * constantDeclarations;

  protected:

    void
    addOpDeclConstActualParameters (SgScopeStatement * scope,
        unsigned int argumentCounter);

    void
    createKernelCallEpilogueStatements (SgScopeStatement * scope);

    SgForStatement *
    createReductionUpdateStatements (unsigned int OP_DAT_ArgumentGroup);

    virtual void
    createReductionEpilogueStatements ();

    SgForStatement *
    createReductionInitialisationStatements (unsigned int OP_DAT_ArgumentGroup);

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionDeclarations ();

    void
    createOpenCLConfigurationLaunchDeclarations ();

    CPPOpenCLHostSubroutine (SgScopeStatement * moduleScope,
        CPPOpenCLKernelSubroutine * calleeSubroutine,
        CPPParallelLoop * parallelLoop,
        CPPModuleDeclarations * moduleDeclarations,
        CPPUserSubroutine * userSubroutine,
        CPPOpenCLConstantDeclarations * constantDeclarations);
};

#endif
