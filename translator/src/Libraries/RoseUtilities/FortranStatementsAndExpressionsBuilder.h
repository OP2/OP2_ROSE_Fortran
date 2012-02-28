


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
#ifndef FORTRAN_STATEMENTS_AND_EXPRESSIONS_BUILDER_H
#define FORTRAN_STATEMENTS_AND_EXPRESSIONS_BUILDER_H

#include <stdarg.h>
#include <string>

class SgVariableDeclaration;
class SgFortranDo;
class SgExpression;
class SgBasicBlock;
class SgScopeStatement;
class SgVarRefExp;
class SgImplicitStatement;
class SgAggregateInitializer;
class SgFunctionParameterList;
class SgType;
class SgExprStatement;

enum FORTRAN_VARIABLE_ATTRIBUTES
{
  ALLOCATABLE,
  TARGET,
  CUDA_CONSTANT,
  CUDA_DEVICE,
  CUDA_SHARED,
  VALUE,
  INTENT_IN,
  INTENT_OUT,
  INTENT_INOUT,
  SAVE
};

class FortranStatementsAndExpressionsBuilder
{
  private:

    static void
    setFortranAttributes (SgVariableDeclaration * variableDeclaration,
        int remainingArguments, va_list attributeArguments);

  public:

    /*
     * ======================================================
     * Builds a Fortran 'do' statement
     * ======================================================
     */

    static SgFortranDo
        *
        buildFortranDoStatement (SgExpression * initialization,
            SgExpression * bound, SgExpression * increment,
            SgBasicBlock * loopBody);

    /*
     * ======================================================
     * Builds a Fortran shape expression for a variable
     * declaration assumed to be an integer
     * ======================================================
     */

    static SgAggregateInitializer *
    buildShapeExpression (SgExpression * expression);

    /*
     * ======================================================
     * Appends a local variable declaration to the given scope.
     * Various Fortran attributes of the variable declaration
     * can be also be set by appending them to the end of the
     * parameter list. When remainingArguments == 0, there
     * are no such attributes; otherwise, remainingArguments
     * dictates how many such attributes have been attached.
     * ======================================================
     */

    static SgVariableDeclaration *
    appendVariableDeclaration (std::string const & variableName, SgType * type,
        SgScopeStatement * scope, int remainingArguments = 0, ...);

    /*
     * ======================================================
     * Appends a variable declaration to the given scope and
     * to the given formal parameter list.
     * Various Fortran attributes of the variable declaration
     * can be also be set by appending them to the end of the
     * parameter list. When remainingArguments == 0, there
     * are no such attributes; otherwise, remainingArguments
     * dictates how many such attributes have been attached.
     * ======================================================
     */

    static SgVariableDeclaration *
    appendVariableDeclarationAsFormalParameter (
        std::string const & variableName, SgType * type,
        SgScopeStatement * scope, SgFunctionParameterList * formalParameters,
        int remainingArguments = 0, ...);

    /*
     * ======================================================
     * Appends an allocate statement to the given scope
     * with size
     * ======================================================
     */

    static void
    appendAllocateStatement (SgVarRefExp * arrayReference,
        SgExpression * size, SgScopeStatement * scope);
        
        
    /*
     * ======================================================
     * Appends an allocate statement to the given scope
     * with lower and upperBounds
     * ======================================================
     */

    static void
    appendAllocateStatement (SgVarRefExp * arrayReference,
        SgExpression * lowerBound, SgExpression * upperBound,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * Appends a deallocate statement to the given scope
     * ======================================================
     */

    static void
    appendDeallocateStatement (SgVarRefExp * arrayReference,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * Builds the 'IMPLICIT NONE' statement
     * ======================================================
     */

    static SgImplicitStatement *
    buildImplicitNoneStatement ();

    /*
     * ======================================================
     * Returns a statement which represents a C-to-Fortran
     * pointer conversion
     * ======================================================
     */

    static SgExprStatement *
    createCToFortranPointerCallStatement (SgScopeStatement * scope,
        SgExpression * parameter1, SgExpression * parameter2,
        SgExpression * parameter3 = NULL);
};

#endif
