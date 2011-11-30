#pragma once
#ifndef CPP_SUBROUTINES_GENERATION_H
#define CPP_SUBROUTINES_GENERATION_H

#include <SubroutinesGeneration.h>

class CPPProgramDeclarationsAndDefinitions;
class CPPHostSubroutine;
class CPPReductionSubroutines;
class CPPModuleDeclarations;
class CPPUserSubroutine;

class CPPSubroutinesGeneration: public SubroutinesGeneration <
    CPPProgramDeclarationsAndDefinitions, CPPHostSubroutine>
{
  protected:

    CPPReductionSubroutines * reductionSubroutines;

    CPPModuleDeclarations * moduleDeclarations;

    std::map <std::string, CPPUserSubroutine *> userSubroutines;

  protected:

    virtual void
    addFreeVariableDeclarations ();

    void
    addOP2IncludeDirective ();

    void
    patchCallsToParallelLoops ();

    void
    generate ();

    virtual void
    addHeaderIncludes () = 0;

    virtual void
    createSubroutines () = 0;

  public:

    CPPSubroutinesGeneration (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations,
        std::string const & newFileName);
};

#endif
