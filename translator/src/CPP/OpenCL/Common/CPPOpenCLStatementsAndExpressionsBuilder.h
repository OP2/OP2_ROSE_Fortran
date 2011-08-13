/*
 * Written by Tiziano Santoro
 *
 * Helper functions to build particular OpenCL statements
 */

#ifndef CPP_OPENCL_STATEMENTS_AND_EXPRESSIONS_BUILDER_H
#define CPP_OPENCL_STATEMENTS_AND_EXPRESSIONS_BUILDER_H

#include <rose.h>
#include <stdarg.h>

#include <CPPStatementsAndExpressionsBuilder.h>

class CPPOpenCLStatementsAndExpressionsBuilder :
  public CPPStatementsAndExpressionsBuilder
{
  public:
    
    static SgExpression *
    generateGetLocalId (
        SgScopeStatement * scope = NULL );
    
    static SgExpression *
    generateGetLocalSize (
        SgScopeStatement * scope = NULL );
    
    static SgExpression *
    generateGetGlobalId (
        SgScopeStatement * scope = NULL );
    
    static SgExpression *
    generateGetGlobalSize (
        SgScopeStatement * scope = NULL );
    
    static SgExpression *
    generateGetGroupId (
        SgScopeStatement * scope = NULL );
    
    static SgExpression *
    generateGetGroupSize (
        SgScopeStatement * scope = NULL );
    
    static SgStatement *
    generateBarrierStatement (
        SgScopeStatement * scope = NULL );
    


};

#endif
