/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class contains additional functions required in
 * Fortran when a plan function is involved
 */

#ifndef FORTRAN_PLAN_H
#define FORTRAN_PLAN_H

class FortranPlan
{
  protected:

    /*
     * ======================================================
     * All the data returned by the plan function is contained
     * in a C interface. This function creates statements to
     * convert that data onto the Fortran side and places
     * the statements into the supplied vector
     * ======================================================
     */
    virtual void
    createStatementsToConvertCPointers (
        std::vector <SgStatement *> & statements) = 0;
};

#endif
