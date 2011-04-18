/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Helper functions to build Fortran types with particular
 * attributes, such as an integer with n bytes
 */

#ifndef FORTRAN_TYPES_BUILDER_H
#define FORTRAN_TYPES_BUILDER_H

#include <rose.h>

class FortranTypesBuilder
{
  private:

    static SgTypeInt * twoByteInteger;

    static SgTypeInt * fourByteInteger;

    static SgTypeInt * eightByteInteger;

    static SgTypeFloat * singlePrecisionFloat;

    static SgTypeFloat * doublePrecisionFloat;

    static std::vector <SgArrayType *> arrays;

  private:

    /*
     * ======================================================
     * A helper function to initiate the construction of a
     * function by setting up ROSE-specific information
     * ======================================================
     */
    static SgFunctionSymbol *
    buildFunctionDeclaration (std::string const & functionName,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * A helper function to complete the construction of a
     * class declaration by setting up ROSE-specific information
     * ======================================================
     */
    static void
    completeNewDeclaration (SgClassDefinition * classDefinition,
        SgClassDeclaration * classDeclaration,
        SgClassDeclaration * nonDefiningClassDeclaration,
        SgScopeStatement * scope);

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
     * Returns a one-dimensional Fortran array of elements
     * of the given type.
     * If lowerBound = -1, this indicates that the lower
     * bound of the array is unknown.
     * If upperBound = -1, this indicates that the upper
     * bound of the array is unknown.
     * ======================================================
     */
    static SgArrayType *
    getArray_RankOne (SgType * baseType, int lowerBound = -1, int upperBound =
        -1);

    /*
     * ======================================================
     * Returns a one-dimensional Fortran array of elements
     * of the given type with this specific lower bound
     * ======================================================
     */
    static SgArrayType *
    getArray_RankOne_WithLowerBound (SgType * baseType,
        SgExpression * lowerBoundExpression);

    /*
     * ======================================================
     * Returns a one-dimensional Fortran array of elements
     * of the given type with this specific upper bound
     * ======================================================
     */
    static SgArrayType *
    getArray_RankOne_WithUpperBound (SgType * baseType,
        SgExpression * upperBoundExpression);

    /*
     * ======================================================
     * Returns a one-dimensional Fortran array of elements
     * of the given type with these specific lower and upper
     * bounds
     * ======================================================
     */
    static SgArrayType *
    getArray_RankOne_WithLowerAndUpperBounds (SgType * baseType,
        SgExpression * lowerBoundExpression,
        SgExpression * upperBoundExpression);

    /*
     * ======================================================
     * Returns a Fortran string of the given length.
     * If length = 1, it is effectively a character
     * ======================================================
     */
    static SgTypeString *
    getString (int length = 1);

    /*
     * ======================================================
     * Creates a new Fortran module declaration and definition
     * in the given scope with the given name
     * ======================================================
     */
    static SgModuleStatement *
    buildNewFortranModuleDeclaration (std::string const & moduleName,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * Creates a new type declaration and definition in the
     * given scope with the given name
     * ======================================================
     */
    static SgClassDeclaration *
    buildNewTypeDeclaration (std::string const & typeName,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * Creates a new Fortran function in the given scope with
     * the given name
     * ======================================================
     */
    static SgFunctionSymbol *
    buildNewFortranFunction (std::string const & functionName,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * Creates a new Fortran subroutine in the given scope with
     * the given name
     * ======================================================
     */
    static SgFunctionSymbol *
    buildNewFortranSubroutine (std::string const & functionName,
        SgScopeStatement * scope);
};

#endif
