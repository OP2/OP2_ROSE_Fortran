/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef CPP_PARALLEL_LOOP_H
#define CPP_PARALLEL_LOOP_H

#include <ParallelLoop.h>
#include <CPPProgramDeclarationsAndDefinitions.h>

class CPPParallelLoop: public ParallelLoop <SgFunctionDeclaration,
    CPPProgramDeclarationsAndDefinitions>
{
  private:

    /*
     * ======================================================
     * The following constants assume that each call to an
     * OP_PAR_LOOP in C++ is constructed as follows:
     *
     * OP_PAR_LOOP_<N>
     *   (nameOfUserSubroutine,
     *    OP_SET,
     *    OP_DAT,
     *    ...
     *    OP_DAT_N)
     * ======================================================
     */

    /*
     * ======================================================
     * There are 2 non-OP_DAT arguments, namely
     * 'nameOfUserSubroutine' and 'OP_SET'
     * ======================================================
     */
    static unsigned int const NUMBER_OF_NON_OP_DAT_ARGUMENTS = 2;

  private:

    virtual void
    handleOpGblDeclaration (OpGblDefinition * opGblDeclaration,
        std::string const & variableName, int opDatArgumentGroup);

    virtual void
    handleOpDatDeclaration (OpDatDefinition * opDatDeclaration,
        std::string const & variableName, int opDatArgumentGroup);

    virtual void
    retrieveOpDatDeclarations (
        CPPProgramDeclarationsAndDefinitions * declarations);

  public:

    virtual void
    generateReductionSubroutines (SgScopeStatement * moduleScope);

    virtual unsigned int
    getNumberOfOpDatArgumentGroups () const;

    CPPParallelLoop (SgFunctionCallExp * functionCallExpression,
        std::string userSubroutineName,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
