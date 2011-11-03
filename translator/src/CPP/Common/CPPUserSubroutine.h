#pragma once
#ifndef CPP_USER_SUBROUTINE_H
#define CPP_USER_SUBROUTINE_H

#include <UserSubroutine.h>

class CPPParallelLoop;
class CPPProgramDeclarationsAndDefinitions;

class CPPUserSubroutine: public UserSubroutine <SgFunctionDeclaration,
    CPPProgramDeclarationsAndDefinitions>
{

  protected:

    void
    forceOutputOfCodeToFile ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPUserSubroutine (SgScopeStatement * moduleScope,
        CPPParallelLoop * parallelLoop,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
