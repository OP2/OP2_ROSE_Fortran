#pragma once
#ifndef CPP_SUBROUTINES_GENERATION_H
#define CPP_SUBROUTINES_GENERATION_H

#include <SubroutinesGeneration.h>

class CPPProgramDeclarationsAndDefinitions;
class CPPHostSubroutine;
class CPPOpDatDimensionsDeclaration;

class CPPSubroutinesGeneration: public SubroutinesGeneration <
    CPPProgramDeclarationsAndDefinitions, CPPHostSubroutine>
{
  protected:

    std::map <std::string, CPPOpDatDimensionsDeclaration *>
        dimensionsDeclarations;

    virtual CPPHostSubroutine *
    createSubroutines () = 0;

    void
    patchCallsToParallelLoops ();

    /*
     * ======================================================
     * Adds the relevant library 'use' statements to the
     * generated module. This function is pure virtual as the
     * needed libraries are backend specific
     * ======================================================
     */
    virtual void
    addLibraries () = 0;

    void
    generate ();

  public:

    CPPSubroutinesGeneration (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations,
        std::string const & newFileName);
};

#endif
