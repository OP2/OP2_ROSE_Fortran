


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#pragma once
#ifndef FORTRAN_TYPES_BUILDER_H
#define FORTRAN_TYPES_BUILDER_H

#include <vector>
#include <string>

class SgTypeBool;
class SgTypeInt;
class SgTypeFloat;
class SgArrayType;
class SgScopeStatement;
class SgClassDeclaration;
class SgClassDefinition;
class SgType;
class SgExpression;
class SgFunctionSymbol;
class SgModuleStatement;
class SgTypeString;

class FortranTypesBuilder
{
  private:

    static SgTypeInt * twoByteIntegerType;

    static SgTypeInt * fourByteIntegerType;

    static SgTypeFloat * singlePrecisionFloatType;

    static SgTypeFloat * doublePrecisionFloatType;

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
     * Effectively a 'short'
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
     * of the given type with this specific number of
     * elements
     * ======================================================
     */

    static SgArrayType *
    getArrayTypePlainDimension (SgType * baseType, int dimension);

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
     * Returns a one-dimensional Fortran array of elements
     * of the given type, where the upper bound can be an
     * expression.
     * If lowerBound = -1, this indicates that the lower
     * bound of the array is unknown.
     * If upperBound = NULL, this indicates that the upper
     * bound of the array is unknown.
     * ======================================================
     */

    static SgArrayType *
    getArray_RankOne (SgType * baseType, int lowerBound,
        SgExpression * upperBoundExpression = NULL);

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
    buildModuleDeclaration (std::string const & moduleName,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * Creates a new type declaration and definition in the
     * given scope with the given name
     * ======================================================
     */

    static SgClassDeclaration *
    buildClassDeclaration (std::string const & typeName,
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
