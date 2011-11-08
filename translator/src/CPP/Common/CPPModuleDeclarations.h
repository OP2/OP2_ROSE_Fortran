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

    std::string const
    getBlockSizeVariableName (std::string const & userSubroutineName);

    std::string const
    getPartitionSizeVariableName (std::string const & userSubroutineName);

    void
    addParallelLoopSettingsVariables (SgScopeStatement * moduleScope,
        CPPProgramDeclarationsAndDefinitions * declarations);

  public:

    SgVarRefExp *
    getBlockSizeReference (std::string const & userSubroutineName);

    SgVarRefExp *
    getPartitionSizeReference (std::string const & userSubroutineName);

    CPPModuleDeclarations (SgScopeStatement * moduleScope,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
