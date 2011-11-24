#pragma once
#ifndef CPP_MODULE_DECLARATIONS_H
#define CPP_MODULE_DECLARATIONS_H

#include <string>

class SgVarRefExp;
class SgScopeStatement;
class ScopedVariableDeclarations;
class CPPProgramDeclarationsAndDefinitions;

class CPPModuleDeclarations
{
  private:

    /*
     * ======================================================
     * Variables declared at module scope level
     * ======================================================
     */

    ScopedVariableDeclarations * variableDeclarations;

  private:

    void
    addParallelLoopSettingsVariables (SgScopeStatement * moduleScope,
        CPPProgramDeclarationsAndDefinitions * declarations);

  public:

    ScopedVariableDeclarations *
    getDeclarations ();

    CPPModuleDeclarations (SgScopeStatement * moduleScope,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
