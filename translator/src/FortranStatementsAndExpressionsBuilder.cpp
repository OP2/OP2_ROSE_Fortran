#include <FortranStatementsAndExpressionsBuilder.h>
#include <ROSEHelper.h>

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgFortranDo *
FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
    SgExpression * initialization, SgExpression * bound,
    SgExpression * increment, SgBasicBlock * loopBody)
{
  using SageInterface::setOneSourcePositionForTransformation;

  SgFortranDo * fortranDoStatement = new SgFortranDo (initialization, bound,
      increment, loopBody);
  fortranDoStatement->setCaseInsensitive (true);
  fortranDoStatement->set_has_end_statement (true);

  setOneSourcePositionForTransformation (fortranDoStatement);

  initialization->set_parent (fortranDoStatement);
  bound->set_parent (fortranDoStatement);
  increment->set_parent (fortranDoStatement);
  loopBody->set_parent (fortranDoStatement);

  return fortranDoStatement;
}

SgIfStmt *
FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
    SgExpression * ifGuard, SgBasicBlock * thenBlock)
{
  using SageBuilder::buildExprStatement;
  using SageInterface::setOneSourcePositionForTransformation;

  SgStatement * ifGuardStatement = buildExprStatement (ifGuard);

  SgIfStmt * ifStatement = new SgIfStmt (ifGuardStatement, thenBlock, NULL);
  ifStatement->setCaseInsensitive (true);
  ifStatement->set_use_then_keyword (true);
  ifStatement->set_has_end_statement (true);

  setOneSourcePositionForTransformation (ifStatement);

  ifGuardStatement->set_parent (ifStatement);
  thenBlock->set_parent (ifStatement);

  return ifStatement;
}

SgExpression *
FortranStatementsAndExpressionsBuilder::buildShapeExpression (
    SgVariableDeclaration * variableDeclaration, SgScopeStatement * scope)
{
  using SageBuilder::buildOpaqueVarRefExp;

  SgExpression * shapeExpression = buildOpaqueVarRefExp ("(/"
      + ROSEHelper::getFirstVariableName (variableDeclaration) + "/)", scope);

  return shapeExpression;
}

void
FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
    SgVariableDeclaration * variableDeclaration, SgExpression * rhs,
    SgScopeStatement * scope)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;

  SgVarRefExp * variableReference = buildVarRefExp (variableDeclaration);

  SgExpression * assignmentExpression = buildAssignOp (variableReference, rhs);

  SgStatement * assignmentStatement = buildExprStatement (assignmentExpression);

  appendStatement (assignmentStatement, scope);
}

SgVariableDeclaration *
FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
    std::string const & variableName, SgType * type, SgScopeStatement * scope)
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, type, NULL, scope);

  variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration, scope);

  return variableDeclaration;
}

void
FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
    SgExprListExp * allocateParameters, SgScopeStatement * scope)
{
  using SageInterface::appendStatement;

  SgAllocateStatement * allocateStatement = new SgAllocateStatement (
      ROSEHelper::getFileInfo ());
  allocateStatement->set_endOfConstruct (ROSEHelper::getFileInfo ());
  allocateStatement->set_expr_list (allocateParameters);

  appendStatement (allocateStatement, scope);
}

void
FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
    SgExprListExp * deallocateParameters, SgScopeStatement * scope)
{
  using SageInterface::appendStatement;

  SgDeallocateStatement * deallocateStatement = new SgDeallocateStatement (
      ROSEHelper::getFileInfo ());
  deallocateStatement->set_endOfConstruct (ROSEHelper::getFileInfo ());
  deallocateStatement->set_expr_list (deallocateParameters);

  appendStatement (deallocateStatement, scope);
}
