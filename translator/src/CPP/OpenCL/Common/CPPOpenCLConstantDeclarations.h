#pragma once
#ifndef CPP_OPENCL_CONSTANT_DECLARATIONS_H
#define CPP_OPENCL_CONSTANT_DECLARATIONS_H

#include <string>

class CPPProgramDeclarationsAndDefinitions;
class ScopedVariableDeclarations;
class SgScopeStatement;

class CPPOpenCLConstantDeclarations
{
  private:

    ScopedVariableDeclarations * variableDeclarations;

  private:

    void
    addDeclarations (CPPProgramDeclarationsAndDefinitions * declarations,
        SgScopeStatement * moduleScope);

  public:

    ScopedVariableDeclarations *
    getDeclarations ();

    CPPOpenCLConstantDeclarations (
        CPPProgramDeclarationsAndDefinitions * declarations,
        SgScopeStatement * moduleScope);

};

#endif
