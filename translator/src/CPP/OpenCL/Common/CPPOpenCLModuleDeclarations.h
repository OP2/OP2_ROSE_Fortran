#pragma once
#ifndef CPP_OpenCL_MODULE_DECLARATIONS_H
#define CPP_OpenCL_MODULE_DECLARATIONS_H

#include <CPPModuleDeclarations.h>

class CPPOpenCLModuleDeclarations: public CPPModuleDeclarations
{
  protected:

    void
    createReductionDeclarations ();

  public:

    SgVariableDeclaration *
    getReductionArrayHostVariableDeclaration ();

    SgVariableDeclaration *
    getReductionArrayDeviceVariableDeclaration ();

    CPPOpenCLModuleDeclarations (std::string const & userSubroutineName,
        CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
