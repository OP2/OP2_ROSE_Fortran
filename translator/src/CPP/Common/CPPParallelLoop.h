/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef CPP_PARALLEL_LOOP_H
#define CPP_PARALLEL_LOOP_H

#include <ParallelLoop.h>

class CPPParallelLoop: public ParallelLoop <SgFunctionDeclaration>
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

  public:

    virtual void
    generateReductionSubroutines (SgScopeStatement * moduleScope);

    virtual unsigned int
    getNumberOfOpDatArgumentGroups ();

    CPPParallelLoop (SgFunctionCallExp * functionCallExpression);
};

#endif
