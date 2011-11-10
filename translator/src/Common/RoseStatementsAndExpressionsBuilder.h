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
