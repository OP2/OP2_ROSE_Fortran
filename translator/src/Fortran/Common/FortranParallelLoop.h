/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_PARALLEL_LOOP_H
#define FORTRAN_PARALLEL_LOOP_H

#include <ParallelLoop.h>
#include <FortranDeclarations.h>

class FortranParallelLoop: public ParallelLoop <SgProcedureHeaderStatement,
    FortranDeclarations>
{
  private:

    virtual void
    handleOpGblDeclaration (OpGblDeclaration * opGblDeclaration,
        std::string const & variableName, int opDatArgumentGroup);

    virtual void
    handleOpDatDeclaration (OpDatDeclaration * opDatDeclaration,
        std::string const & variableName, int opDatArgumentGroup);

    virtual void
    retrieveOpDatDeclarations (FortranDeclarations * declarations);

  public:

    virtual void
    generateReductionSubroutines (SgScopeStatement * moduleScope);

    FortranParallelLoop (SgExpressionPtrList & actualArguments,
        std::string userSubroutineName, FortranDeclarations * declarations);
};

#endif
