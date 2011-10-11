#pragma once
#ifndef CPP_OPENCL_REDUCTION_SUBROUTINE_H
#define CPP_OPENCL_REDUCTION_SUBROUTINE_H

#include <Subroutine.h>

class Reduction;

class CPPOpenCLReductionSubroutine: public Subroutine <
    SgProcedureHeaderStatement>
{
  private:

    /*
     * ======================================================
     * Information pertaining to the type of reduction
     * ======================================================
     */
    Reduction * reduction;

    std::string autosharedVariableName;

  private:

    SgStatement *
    createSynchThreadsCallStatement ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPOpenCLReductionSubroutine (
        std::string const & subroutineAndVariableName,
        SgScopeStatement * moduleScope, Reduction * reduction);
};

#endif

