/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef FORTRAN_DECLARATIONS_H
#define FORTRAN_DECLARATIONS_H

#include <rose.h>
#include <Declarations.h>

class FortranDeclarations: public Declarations <SgProcedureHeaderStatement> ,
    public AstSimpleProcessing
{
  private:

    virtual void
    visit (SgNode * node);

  public:

    FortranDeclarations (SgProject * project);
};

#endif
