#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>

SgIfStmt *
RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
    SgExpression * ifGuard, SgScopeStatement * thenBlock)
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

SgDerivedTypeStatement *
RoseStatementsAndExpressionsBuilder::buildTypeDeclaration (
    std::string const & typeName, SgScopeStatement * scope)
{
  SgClassDefinition * classDefinition = new SgClassDefinition (
      RoseHelper::getFileInfo ());
  classDefinition->set_endOfConstruct (RoseHelper::getFileInfo ());
  classDefinition->setCaseInsensitive (true);

  SgDerivedTypeStatement* classDeclaration = new SgDerivedTypeStatement (
      RoseHelper::getFileInfo (), typeName, SgClassDeclaration::e_struct, NULL,
      classDefinition);
  classDeclaration->set_endOfConstruct (RoseHelper::getFileInfo ());
  classDeclaration->set_definingDeclaration (classDeclaration);
  classDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  SgDerivedTypeStatement* nondefiningClassDeclaration =
      new SgDerivedTypeStatement (RoseHelper::getFileInfo (), typeName,
          SgClassDeclaration::e_struct, NULL, NULL);
  nondefiningClassDeclaration->set_endOfConstruct (RoseHelper::getFileInfo ());
  nondefiningClassDeclaration->set_parent (scope);
  nondefiningClassDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  nondefiningClassDeclaration->set_type (SgClassType::createType (
      nondefiningClassDeclaration));
  classDeclaration->set_type (nondefiningClassDeclaration->get_type ());

  classDeclaration->set_firstNondefiningDeclaration (
      nondefiningClassDeclaration);

  nondefiningClassDeclaration->set_firstNondefiningDeclaration (
      nondefiningClassDeclaration);
  nondefiningClassDeclaration->set_definingDeclaration (classDeclaration);

  nondefiningClassDeclaration->setForward ();

  classDefinition->set_declaration (classDeclaration);
  classDefinition->get_declaration ()->get_declarationModifier ().get_accessModifier ().setUndefined ();

  classDeclaration->set_scope (scope);
  nondefiningClassDeclaration->set_scope (scope);

  classDeclaration->set_parent (scope);

  SgClassSymbol * classSymbol = new SgClassSymbol (nondefiningClassDeclaration);

  scope->insert_symbol (typeName, classSymbol);

  return classDeclaration;
}

SgVariableDeclaration *
RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
    std::string const & variableName, SgType * type, SgScopeStatement * scope)
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, type, NULL, scope);

  appendStatement (variableDeclaration, scope);

  return variableDeclaration;
}

SgVariableDeclaration *
RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
    std::string const & variableName, SgType * type, SgScopeStatement * scope,
    SgFunctionParameterList * formalParameters)
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, type, NULL, scope);

  formalParameters->append_arg (
      *(variableDeclaration->get_variables ().begin ()));

  appendStatement (variableDeclaration, scope);

  return variableDeclaration;
}
