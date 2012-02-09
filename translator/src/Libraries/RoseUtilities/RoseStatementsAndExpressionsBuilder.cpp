


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


#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <rose.h>

SgIfStmt *
RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
    SgExpression * ifGuard, SgScopeStatement * thenBlock)
{
  using namespace SageBuilder;
  using namespace SageInterface;

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
  using namespace SageBuilder;
  using namespace SageInterface;

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
  using namespace SageBuilder;
  using namespace SageInterface;

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, type, NULL, scope);

  formalParameters->append_arg (
      *(variableDeclaration->get_variables ().begin ()));

  return variableDeclaration;
}
