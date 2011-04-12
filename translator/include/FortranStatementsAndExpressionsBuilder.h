/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Helper functions to build particular kinds of Fortran statements
 * and expressions
 */

#ifndef FORTRAN_STATEMENTS_AND_EXPRESSIONS_BUILDER_H
#define FORTRAN_STATEMENTS_AND_EXPRESSIONS_BUILDER_H

#include <rose.h>

class FortranStatementsAndExpressionsBuilder
{
  public:

    /*
     * ======================================================
     * Builds a Fortran 'do' construct
     * ======================================================
     */
    static SgFortranDo
        *
        buildFortranDoStatement (SgExpression * initialization,
            SgExpression * bound, SgExpression * increment,
            SgBasicBlock * loopBody);

    /*
     * ======================================================
     * Builds an 'if' construct which has an empty 'else'
     * ======================================================
     */
    static SgIfStmt *
    buildIfStatementWithEmptyElse (SgExpression * ifGuard,
        SgBasicBlock * thenBlock);

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
     * Builds a Fortran shape expression for a call to
     * 'c_f_pointer'
     * ======================================================
     */
    static SgExpression *
    buildShapeExpression (SgVariableDeclaration * variableDeclaration,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * Appends an assignment statement to the given scope
     * ======================================================
     */
    static void
    appendAssignmentStatement (SgVariableDeclaration * variableDeclaration,
        SgExpression * rhs, SgScopeStatement * scope);

    /*
     * ======================================================
     * Appends a local variable declaration to the given scope
     * and returns it so other properties can be set
     * ======================================================
     */
    static SgVariableDeclaration *
    appendVariableDeclaration (std::string const & variableName, SgType * type,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * Appends an allocate statement to the given scope
     * ======================================================
     */
    static void
    appendAllocateStatement (SgExprListExp * allocateParameters,
        SgScopeStatement * scope);

    /*
     * ======================================================
     * Appends a deallocate statement to the given scope
     * ======================================================
     */
    static void
    appendDeallocateStatement (SgExprListExp * deallocateParameters,
        SgScopeStatement * scope);
};

#endif
