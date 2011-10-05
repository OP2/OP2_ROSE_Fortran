#pragma once
#ifndef CPP_MODULE_DECLARATIONS_H
#define CPP_MODULE_DECLARATIONS_H

#include <string>

class CPPModuleDeclarations
{
  protected:

    /*
     * ======================================================
     * The name of the user subroutine
     * ======================================================
     */
    std::string userSubroutineName;

    /*
     * ======================================================
     * Variables declared at module scope level
     * ======================================================
     */
    SubroutineVariableDeclarations * variableDeclarations;

    /*
     * ======================================================
     * The parallel loop
     * ======================================================
     */
    CPPParallelLoop * parallelLoop;

    /*
     * ======================================================
     * The module scope
     * ======================================================
     */
    SgScopeStatement * moduleScope;

  protected:

    void
    createInitialiseConstantsBooleanDeclaration ();

    void
    createFirstExecutionBooleanDeclaration ();

    CPPModuleDeclarations (std::string const & userSubroutineName,
        CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope);

  public:

    SgVariableDeclaration *
    getInitialiseConstantsBooleanDeclaration ();

    SgVariableDeclaration *
    getFirstExecutionBooleanDeclaration ();

    SgVariableDeclaration *
    getGlobalOpDatDeclaration (unsigned int OP_DAT_ArgumentGroup);

    SubroutineVariableDeclarations *
    getAllDeclarations ();
};

#endif
