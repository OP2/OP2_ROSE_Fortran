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

    CPPHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif
