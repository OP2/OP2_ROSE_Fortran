/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H
#define FORTRAN_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H

#include <rose.h>
#include <ProgramDeclarationsAndDefinitions.h>

class FortranProgramDeclarationsAndDefinitions: public ProgramDeclarationsAndDefinitions <
    SgProcedureHeaderStatement> ,
    public AstSimpleProcessing
{
  private:

    virtual void
    visit (SgNode * node);

  public:

    FortranProgramDeclarationsAndDefinitions (SgProject * project);
};

#endif
