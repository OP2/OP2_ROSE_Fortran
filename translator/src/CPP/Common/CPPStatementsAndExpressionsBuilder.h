/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Helper functions to build particular kinds of CPP statements
 * and expressions
 */

#ifndef CPP_STATEMENTS_AND_EXPRESSIONS_BUILDER_H
#define CPP_STATEMENTS_AND_EXPRESSIONS_BUILDER_H

#include <rose.h>
#include <stdarg.h>

enum CPP_VARIABLE_ATTRIBUTES
{
  CONSTANT,
  DEVICE,
  SHARED
};

class CPPStatementsAndExpressionsBuilder
{
  private:

    static void
    setCPPAttributes (SgVariableDeclaration * variableDeclaration,
        int remainingArguments, va_list attributeArguments);

  public:


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
    appendVariableDeclaration (
        std::string const & variableName, 
        SgType * type,
        SgScopeStatement * scope, 
        int remainingArguments = 0, 
        ...);

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
        std::string const & variableName, 
        SgType * type,
        SgScopeStatement * scope, 
        SgFunctionParameterList * formalParameters,
        int remainingArguments = 0, ...);

 

    /*
     * ======================================================
     * Returns the CPP kind of an OP_DAT type or a default
     * one (in case of types without kind)
     * ======================================================
     */
    static SgExpression *
    getCPPKindOfOpDat (SgType * OpDatBaseType);

};

#endif
