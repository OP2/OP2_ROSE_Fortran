/*
 * Written by Carlo Bertolli and Adam Betts
 *
 * This class generates code implementing reduction of OP_GBL variables marked
 * with OP_INC, OP_MAX or OP_MIN access
 */

#ifndef FORTRAN_CUDA_REDUCTION_SUBROUTINE_H
#define FORTRAN_CUDA_REDUCTION_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

class FortranCUDAReductionSubroutine: public Subroutine
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

    FortranCUDAReductionSubroutine (std::string const & subroutineAndVariableName,
        SgScopeStatement * moduleScope, SgArrayType * reductionVariableType);
};

#endif

