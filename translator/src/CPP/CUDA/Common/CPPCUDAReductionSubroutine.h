#pragma once
#ifndef CPP_CUDA_REDUCTION_SUBROUTINE_H
#define CPP_CUDA_REDUCTION_SUBROUTINE_H

#include <Subroutine.h>

class Reduction;

class CPPCUDAReductionSubroutine: public Subroutine <SgFunctionDeclaration>
{
  private:

    /*
     * ======================================================
     * Information pertaining to the type of reduction
     * ======================================================
     */
    Reduction * reduction;

    /*
     * ======================================================
     * There is one shared memory variable per reduction
     * subroutine. And also a volatile variable to comply
     * with the Fermi architecture
     * ======================================================
     */
    std::string sharedVariableName;

    std::string volatileSharedVariableName;

  private:

    void
    createThreadZeroReductionStatements ();

    void
    createFirstRoundOfReduceStatements ();

    void
    createSecondRoundOfReduceStatements ();

    void
    createSharedVariableInitialisationStatements ();

    void
    createInitialisationStatements ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    CPPCUDAReductionSubroutine (SgScopeStatement * moduleScope,
        Reduction * reduction);
};

#endif
