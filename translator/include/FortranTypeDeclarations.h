/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Helper functions to build Fortran types with particular
 * attributes, such as an integer with n bytes
 */

#include "rose.h"

class FortranTypeDeclarations
{
  private:

    static SgModifierType * twoByteInteger;

    static SgModifierType * fourByteInteger;

    static SgModifierType * eightByteInteger;

    static SgModifierType * singlePrecisionFloat;

    static SgModifierType * doublePrecisionFloat;

  public:

    static SgModifierType *
    getTwoByteInteger ();

    static SgModifierType *
    getFourByteInteger ();

    static SgModifierType *
    getEightByteInteger ();

    static SgModifierType *
    getSinglePrecisionFloat ();

    static SgModifierType *
    getDoublePrecisionFloat ();
};
