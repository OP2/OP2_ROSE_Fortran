


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
#ifndef ROSE_STATEMENTS_AND_EXPRESSIONS_BUILDER_H
#define ROSE_STATEMENTS_AND_EXPRESSIONS_BUILDER_H

#include <string>

class SgExpression;
class SgScopeStatement;
class SgVariableDeclaration;
class SgType;
class SgFunctionParameterList;
class SgIfStmt;
class SgDerivedTypeStatement;

class RoseStatementsAndExpressionsBuilder
{
  public:

    /*
     * ======================================================
     * Builds an 'if' construct which has an empty 'else'
     * ======================================================
     */
    static SgIfStmt *
    buildIfStatementWithEmptyElse (SgExpression * ifGuard,
        SgScopeStatement * thenBlock);

    /*
     * ======================================================
     * Builds a type declaration
     * ======================================================
     */
    static SgDerivedTypeStatement *
    buildTypeDeclaration (std::string const & typeName,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * Appends a local variable declaration to the given scope
     * ======================================================
     */
    static SgVariableDeclaration *
    appendVariableDeclaration (std::string const & variableName, SgType * type,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * Appends a variable declaration to the given scope and
     * to the given formal parameter list
     * ======================================================
     */
    static SgVariableDeclaration *
    appendVariableDeclarationAsFormalParameter (
        std::string const & variableName, SgType * type,
        SgScopeStatement * scope, SgFunctionParameterList * formalParameters);
};

#endif
