#pragma once
#ifndef CPP_SUBROUTINES_GENERATION_H
#define CPP_SUBROUTINES_GENERATION_H

#include <SubroutinesGeneration.h>

class CPPProgramDeclarationsAndDefinitions;
class CPPHostSubroutine;
class CPPReductionSubroutines;

class CPPSubroutinesGeneration: public SubroutinesGeneration <
    CPPProgramDeclarationsAndDefinitions, CPPHostSubroutine>
{
  protected:

    CPPReductionSubroutines * reductionSubroutines;

  protected:

    virtual void
    addHeaderIncludes () = 0;

    virtual void
    createSubroutines () = 0;

    void
    patchCallsToParallelLoops ();

    void
    generate ();

  public:

    CPPSubroutinesGeneration (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations,
        std::string const & newFileName);
};

#endif
