/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H
#define FORTRAN_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H

#include <ProgramDeclarationsAndDefinitions.h>
#include <FortranParallelLoop.h>

class FortranProgramDeclarationsAndDefinitions: public ProgramDeclarationsAndDefinitions <
    FortranParallelLoop, SgProcedureHeaderStatement> ,
    public AstSimpleProcessing
{
  private:

    void
    handleOpGblDeclaration (FortranParallelLoop * parallelLoop,
        std::string const & variableName, int opDatArgumentGroup);
    void
    handleOpDatDeclaration (FortranParallelLoop * parallelLoop,
        std::string const & variableName, int opDatArgumentGroup);

    void
    retrieveOpDatDeclarations (FortranParallelLoop * parallelLoop,
        SgExpressionPtrList & actualArguments);

    virtual void
    visit (SgNode * node);

  public:

    FortranProgramDeclarationsAndDefinitions (SgProject * project);
};

#endif
