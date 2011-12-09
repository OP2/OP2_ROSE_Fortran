#pragma once
#ifndef CPP_OPENMP_SUBROUTINES_GENERATION_H
#define CPP_OPENMP_SUBROUTINES_GENERATION_H

#include <CPPSubroutinesGeneration.h>

class CPPOpenMPSubroutinesGeneration: public CPPSubroutinesGeneration
{
  private:

    virtual void
    addFreeVariableDeclarations ();

    virtual void
    addHeaderIncludes ();

    virtual void
    createSubroutines ();

  public:

    CPPOpenMPSubroutinesGeneration (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations);

};

#endif
