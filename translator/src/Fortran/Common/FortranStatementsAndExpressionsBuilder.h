/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Helper functions to build particular kinds of Fortran statements
 * and expressions
 */

#ifndef FORTRAN_STATEMENTS_AND_EXPRESSIONS_BUILDER_H
#define FORTRAN_STATEMENTS_AND_EXPRESSIONS_BUILDER_H

#include <rose.h>
#include <stdarg.h>

class FortranStatementsAndExpressionsBuilder
{
  private:

    static void
    setFortranAttributes (SgVariableDeclaration * variableDeclaration,
        int remainingArguments, va_list attributeArguments);

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
     * Builds a Fortran shape expression for a call to
     * 'c_f_pointer'
     * ======================================================
     */
    static SgExpression *
    buildShapeExpression (SgVariableDeclaration * variableDeclaration,
        SgScopeStatement * scope);

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

    /*
     * ======================================================
     * Returns the Fortran kind of an OP_DAT type or a default
     * one (in case of types without kind)
     * ======================================================
     */
    static SgExpression *
    getFortranKindOfOpDat (SgType * OpDatBaseType);

    /*
     * ======================================================
     * Builds the 'IMPLICIT NONE' statement
     * ======================================================
     */
    static SgImplicitStatement *
    buildImplicitNoneStatement ();
};

#endif
