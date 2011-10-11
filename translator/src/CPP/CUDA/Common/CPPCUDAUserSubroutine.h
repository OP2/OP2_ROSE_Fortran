#pragma once
#ifndef CPP_CUDA_USER_SUBROUTINE_H
#define CPP_CUDA_USER_SUBROUTINE_H

#include <UserSubroutine.h>

class CPPParallelLoop;
class CPPProgramDeclarationsAndDefinitions;

class CPPCUDAUserSubroutine: public UserSubroutine <SgFunctionDeclaration,
    CPPProgramDeclarationsAndDefinitions>
{
  private:

    void
    forceOutputOfCodeToFile ();

    void
    findOriginalSubroutine ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPCUDAUserSubroutine (SgScopeStatement * moduleScope,
        CPPParallelLoop * parallelLoop,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
