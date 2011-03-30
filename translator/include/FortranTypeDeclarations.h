/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Helper functions to build Fortran types with particular
 * attributes, such as an integer with n bytes
 */

#ifndef FORTRAN_TYPE_DECLARATIONS_H
#define FORTRAN_TYPE_DECLARATIONS_H

#include <rose.h>

class FortranTypeDeclarations
{
  private:

    static Sg_File_Info * fileInfo;

    static SgTypeInt * twoByteInteger;

    static SgTypeInt * fourByteInteger;

    static SgTypeInt * eightByteInteger;

    static SgTypeFloat * singlePrecisionFloat;

    static SgTypeFloat * doublePrecisionFloat;

    static std::vector <SgArrayType *> arrays;

    /*
     * ======================================================
     * A helper function to construct the correct type of
     * Fortran array
     * ======================================================
     */
    static SgArrayType *
    getArray_RankOne (int lowerBound, int upperBound, SgType * baseType);

  public:

    /*
     * ======================================================
     * Returns a Fortran integer with 2-byte storage.
     * Effectively a 'short int'
     * ======================================================
     */
    static SgTypeInt *
    getTwoByteInteger ();

    /*
     * ======================================================
     * Returns a Fortran integer with 4-byte storage.
     * Effectively an 'int'
     * ======================================================
     */
    static SgTypeInt *
    getFourByteInteger ();

    /*
     * ======================================================
     * Returns a Fortran integer with 8-byte storage.
     * Effectively a 'long int'
     * ======================================================
     */
    static SgTypeInt *
    getEightByteInteger ();

    /*
     * ======================================================
     * Returns a single-precision Fortran floating-point number
     * ======================================================
     */
    static SgTypeFloat *
    getSinglePrecisionFloat ();

    /*
     * ======================================================
     * Returns a double-precision Fortran floating-point number
     * ======================================================
     */
    static SgTypeFloat *
    getDoublePrecisionFloat ();

    /*
     * ======================================================
     * Returns a one-dimensional Fortran array containing 2-byte
     * integers.
     * If lowerBound = -1, this indicates that the lower
     * bound of the array is unknown.
     * If upperBound = -1, this indicates that the upper
     * bound of the array is unknown.
     * ======================================================
     */
    static SgArrayType *
    getTwoByteIntegerArray_RankOne (int lowerBound = -1, int upperBound = -1);

    /*
     * ======================================================
     * Returns a one-dimensional Fortran array containing 4-byte
     * integers.
     * If lowerBound = -1, this indicates that the lower
     * bound of the array is unknown.
     * If upperBound = -1, this indicates that the upper
     * bound of the array is unknown.
     * ======================================================
     */
    static SgArrayType *
    getFourByteIntegerArray_RankOne (int lowerBound = -1, int upperBound = -1);

    /*
     * ======================================================
     * Returns a one-dimensional Fortran array containing 8-byte
     * integers.
     * If lowerBound = -1, this indicates that the lower
     * bound of the array is unknown.
     * If upperBound = -1, this indicates that the upper
     * bound of the array is unknown.
     * ======================================================
     */
    static SgArrayType *
    getEightByteIntegerArray_RankOne (int lowerBound = -1, int upperBound = -1);

    /*
     * ======================================================
     * Returns a one-dimensional Fortran array containing
     * single-precision floating-point numbers.
     * If lowerBound = -1, this indicates that the lower
     * bound of the array is unknown.
     * If upperBound = -1, this indicates that the upper
     * bound of the array is unknown.
     * ======================================================
     */
    static SgArrayType *
    getSinglePrecisionFloatArray_RankOne (int lowerBound = -1, int upperBound =
        -1);

    /*
     * ======================================================
     * Returns a one-dimensional Fortran array containing
     * double-precision floating-point numbers.
     * If lowerBound = -1, this indicates that the lower
     * bound of the array is unknown.
     * If upperBound = -1, this indicates that the upper
     * bound of the array is unknown.
     * ======================================================
     */
    static SgArrayType *
    getDoublePrecisionFloatArray_RankOne (int lowerBound = -1, int upperBound =
        -1);

    /*
     * ======================================================
     * Returns a Fortran string of the given length.
     * If length = 1, it is effectively a character
     * ======================================================
     */
    static SgTypeString *
    getString (int length = 1);
};

#endif
