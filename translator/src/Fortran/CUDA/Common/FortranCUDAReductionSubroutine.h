/*
 * Written by Carlo Bertolli and Adam Betts
 *
 * This class generates code implementing reduction of OP_GBL variables marked
 * with OP_INC, OP_MAX or OP_MIN access
 */

#ifndef FORTRAN_CUDA_REDUCTION_SUBROUTINE_H
#define FORTRAN_CUDA_REDUCTION_SUBROUTINE_H

#include <Subroutine.h>
#include <Reduction.h>

class FortranCUDAReductionSubroutine: public Subroutine <
    SgProcedureHeaderStatement>
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
     * There is one autoshared variable per reduction
     * subroutine. This stores the name of that variable.
     * The name is constructed from the base type of the OP_DAT
     * variable and the size of the OP_DAT base type
     * ======================================================
     */
    std::string autosharedVariableName;

  private:

    void
    createThreadZeroReductionStatements ();

    void
    createReductionStatements ();

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

    FortranCUDAReductionSubroutine (
        std::string const & subroutineAndVariableName,
        SgScopeStatement * moduleScope, Reduction * reduction);
};

#endif

