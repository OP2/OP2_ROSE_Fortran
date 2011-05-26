/*
 * Written by Carlo Bertolli and Adam Betts
 *
 * This class generates code implementing reduction of OP_GBL variables marked
 * with OP_INC, OP_MAX or OP_MIN access
 */

#ifndef REDUCTION_SUBROUTINE_H
#define REDUCTION_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

class ReductionSubroutine: public Subroutine
{
  private:

    /*
     * ======================================================
     * Type of the reduction variable (must be an array type)
     * ======================================================
     */
    SgArrayType * reductionVariableType;

  private:

    /*
     * ======================================================
     * Generates a call statement to synchthreads
     * ======================================================
     */
    SgStatement *
    createCallToSynchThreads ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    ReductionSubroutine (std::string const & subroutineAndVariableName,
        SgScopeStatement * moduleScope, SgArrayType * reductionVariableType);
};

#endif

