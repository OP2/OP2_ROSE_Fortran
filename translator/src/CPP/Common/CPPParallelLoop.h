/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef CPP_PARALLEL_LOOP_H
#define CPP_PARALLEL_LOOP_H

#include <ParallelLoop.h>
#include <CPPDeclarations.h>

class CPPParallelLoop: public ParallelLoop <SgFunctionDeclaration,
    CPPDeclarations>
{
  private:

    virtual void
    handleOpGblDeclaration (OpGblDefinition * opGblDeclaration,
        std::string const & variableName, int opDatArgumentGroup);

    virtual void
    handleOpDatDeclaration (OpDatDefinition * opDatDeclaration,
        std::string const & variableName, int opDatArgumentGroup);

    virtual void
    retrieveOpDatDeclarations (CPPDeclarations * declarations);

  public:

    virtual void
    generateReductionSubroutines (SgScopeStatement * moduleScope);

    virtual unsigned int
    getNumberOfOpDatArgumentGroups () const;

    CPPParallelLoop (SgExpressionPtrList & actualArguments,
        std::string userSubroutineName, CPPDeclarations * declarations);
};

#endif
