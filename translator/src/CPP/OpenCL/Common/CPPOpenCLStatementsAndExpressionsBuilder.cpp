/*
 * Written by Tiziano Santoro
 *
 * Helper functions to build particular OpenCL statements
 */
#include <CPPOpenCLStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>

using namespace SageBuilder;

SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalId (
    SgScopeStatement * scope )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getLocalId,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalSize (
    SgScopeStatement * scope )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getLocalSize,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetGlobalId (
    SgScopeStatement * scope )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getGlobalId,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetGlobalSize (
    SgScopeStatement * scope )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getGlobalSize,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetGroupId (
    SgScopeStatement * scope )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getGroupId,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetGroupSize (
    SgScopeStatement * scope )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getGroupSize,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

SgStatement *
CPPOpenCLStatementsAndExpressionsBuilder::generateBarrierStatement (
    SgScopeStatement * scope )
{
  SgStatement * functionCallExp = buildFunctionCallStmt(
      OpenCL::CPP::barrier,
      buildIntType(),
      buildExprListExp(
        buildOpaqueVarRefExp(
            "CLK_LOCAL_MEM_FENCE",
            scope ) ),
      scope );
  
  return functionCallExp;
}
