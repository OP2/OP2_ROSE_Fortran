#pragma once
#ifndef CPP_OPENCL_REDUCTION_SUBROUTINE_H
#define CPP_OPENCL_REDUCTION_SUBROUTINE_H

#include <Subroutine.h>

class Reduction;

class CPPOpenCLReductionSubroutine: public Subroutine <SgFunctionDeclaration>
{
  private:

    Reduction * reduction;

  private:

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPOpenCLReductionSubroutine (SgScopeStatement * moduleScope,
        Reduction * reduction);
};

#endif

