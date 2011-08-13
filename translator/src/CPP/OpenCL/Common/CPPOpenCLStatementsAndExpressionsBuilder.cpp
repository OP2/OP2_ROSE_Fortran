/*
 * Written by Tiziano Santoro
 *
 * Helper functions to build particular OpenCL statements
 */

using namespace SageBuilder;

static SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalId (
    SgScopeStatement * scope = NULL )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getLocalId,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

static SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalSize (
    SgScopeStatement * scope = NULL )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getLocalSize,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

static SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetGlobalId (
    SgScopeStatement * scope = NULL )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getGlobalId,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

static SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetGlobalSize (
    SgScopeStatement * scope = NULL )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getGlobalSize,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

static SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetGroupId (
    SgScopeStatement * scope = NULL )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getGroupId,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

static SgExpression *
CPPOpenCLStatementsAndExpressionsBuilder::generateGetGroupSize (
    SgScopeStatement * scope = NULL )
{
  SgFunctionCallExp * functionCallExp = buildFunctionCallExp(
      OpenCL::CPP::getGroupSize,
      buildIntType(),
      buildExprListExp(
          buildIntVal(0) ),
      scope );
  
  return functionCallExp;
}

static SgStatement *
CPPOpenCLStatementsAndExpressionsBuilder::generateBarrierStatement (
    SgScopeStatement * scope = NULL )
{
  SgStatement * functionCallExp = buildFunctionCallStmt(
      OpenCL::CPP::barrier,
      buildIntType(),
      buildOpaqueVarRefExp(
          "CLK_LOCAL_MEM_FENCE",
          scope ),
      scope );
  
  return functionCallExp;
}
