/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef CPP_DECLARATIONS_H
#define CPP_DECLARATIONS_H

#include <Declarations.h>

class CPPDeclarations: public Declarations <SgFunctionDeclaration> ,
    public AstSimpleProcessing
{
  private:

    virtual void
    visit (SgNode * node);

  public:

    CPPDeclarations (SgProject * project);
};

#endif
