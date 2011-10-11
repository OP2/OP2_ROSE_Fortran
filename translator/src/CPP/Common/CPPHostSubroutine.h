#pragma once
#ifndef CPP_HOST_SUBROUTINE_H
#define CPP_HOST_SUBROUTINE_H

#include <HostSubroutine.h>

class CPPParallelLoop;

class CPPHostSubroutine: public HostSubroutine <SgFunctionDeclaration>
{
  protected:

    virtual void
    createFormalParameterDeclarations ();

    CPPHostSubroutine (SgScopeStatement * moduleScope, Subroutine <
        SgFunctionDeclaration> * calleeSubroutine,
        CPPParallelLoop * parallelLoop);
};

#endif
