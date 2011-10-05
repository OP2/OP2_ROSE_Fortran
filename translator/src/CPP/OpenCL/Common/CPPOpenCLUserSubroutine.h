#pragma once
#ifndef CPP_CUDA_USER_SUBROUTINE_H
#define CPP_CUDA_USER_SUBROUTINE_H

#include <UserSubroutine.h>

class CPPParallelLoop;
class CPPProgramDeclarationsAndDefinitions;

class CPPOpenCLUserSubroutine: public UserSubroutine <SgFunctionDeclaration,
    CPPProgramDeclarationsAndDefinitions>
{
  private:

    void
    patchReferencesToConstants ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPOpenCLUserSubroutine (std::string const & subroutineName,
        SgScopeStatement * moduleScope,
        CPPProgramDeclarationsAndDefinitions * declarations,
        CPPParallelLoop * parallelLoop);
};

#endif
