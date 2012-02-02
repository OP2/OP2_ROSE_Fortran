


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


#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "RoseHelper.h"
#include "Exceptions.h"
#include <rose.h>

void
FortranStatementsAndExpressionsBuilder::setFortranAttributes (
    SgVariableDeclaration * variableDeclaration, int remainingArguments,
    va_list attributeArguments)
{
  /*
   * ======================================================
   * The access modifier to the variable declaration is
   * always undefined (not private, public, protected)
   * ======================================================
   */

  variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  for (int i = 0; i < remainingArguments; ++i)
  {
    int attribute = va_arg(attributeArguments, int);

    switch (attribute)
    {
      case ALLOCATABLE:
      {
        variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();
        break;
      }

      case TARGET:
      {
        variableDeclaration->get_declarationModifier ().get_typeModifier ().setTarget ();
        break;
      }

      case CUDA_CONSTANT:
      {
        variableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaConstant ();
        break;
      }

      case CUDA_DEVICE:
      {
        variableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaDevice ();
        break;
      }

      case CUDA_SHARED:
      {
        variableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaShared ();
        break;
      }

      case VALUE:
      {
        variableDeclaration->get_declarationModifier ().get_typeModifier ().setValue ();
        break;
      }

      case INTENT_IN:
      {
        variableDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
        break;
      }

      case INTENT_OUT:
      {
        variableDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_out ();
        break;
      }

      case INTENT_INOUT:
      {
        variableDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_inout ();
        break;
      }

      default:
      {
        throw Exceptions::CodeGeneration::FortranVariableAttributeException (
            "Unhandled Fortran attribute");
      }
    }
  }
}

SgFortranDo *
FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
    SgExpression * initialization, SgExpression * bound,
    SgExpression * increment, SgBasicBlock * loopBody)
{
  using namespace SageInterface;

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

SgAggregateInitializer *
FortranStatementsAndExpressionsBuilder::buildShapeExpression (
    SgExpression * expression)
{
  using namespace SageBuilder;

  SgExprListExp * parameters = buildExprListExp (expression);

  SgAggregateInitializer * shapeExpression = buildAggregateInitializer (
      parameters, buildIntType ());

  return shapeExpression;
}

SgVariableDeclaration *
FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
    std::string const & variableName, SgType * type, SgScopeStatement * scope,
    int remainingArguments, ...)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, type, NULL, scope);

  appendStatement (variableDeclaration, scope);

  va_list fortranAttributes;

  va_start (fortranAttributes, remainingArguments);

  setFortranAttributes (variableDeclaration, remainingArguments,
      fortranAttributes);

  va_end (fortranAttributes);

  return variableDeclaration;
}

SgVariableDeclaration *
FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
    std::string const & variableName, SgType * type, SgScopeStatement * scope,
    SgFunctionParameterList * formalParameters, int remainingArguments, ...)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, type, NULL, scope);

  formalParameters->append_arg (
      *(variableDeclaration->get_variables ().begin ()));

  appendStatement (variableDeclaration, scope);

  va_list fortranAttributes;

  va_start (fortranAttributes, remainingArguments);

  setFortranAttributes (variableDeclaration, remainingArguments,
      fortranAttributes);

  va_end (fortranAttributes);

  return variableDeclaration;
}

void
FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
    SgVarRefExp * arrayReference, SgExpression * size, SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (arrayReference,
      size );

  SgExprListExp * actualParameters = buildExprListExp (arrayExpression);

  SgAllocateStatement * allocateStatement = new SgAllocateStatement (
      RoseHelper::getFileInfo ());
  allocateStatement->set_endOfConstruct (RoseHelper::getFileInfo ());
  allocateStatement->set_expr_list (actualParameters);

  appendStatement (allocateStatement, scope);
}


void
FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
    SgVarRefExp * arrayReference, SgExpression * lowerBound,
    SgExpression * upperBound, SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  SgSubscriptExpression * subscriptExpression = new SgSubscriptExpression (
      RoseHelper::getFileInfo (), lowerBound, upperBound, buildIntVal (1));
  subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (arrayReference,
      subscriptExpression);

  SgExprListExp * actualParameters = buildExprListExp (arrayExpression);

  SgAllocateStatement * allocateStatement = new SgAllocateStatement (
      RoseHelper::getFileInfo ());
  allocateStatement->set_endOfConstruct (RoseHelper::getFileInfo ());
  allocateStatement->set_expr_list (actualParameters);

  appendStatement (allocateStatement, scope);
}

void
FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
    SgVarRefExp * arrayReference, SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  SgExprListExp * actualParameters = buildExprListExp (arrayReference);

  SgDeallocateStatement * deallocateStatement = new SgDeallocateStatement (
      RoseHelper::getFileInfo ());
  deallocateStatement->set_endOfConstruct (RoseHelper::getFileInfo ());
  deallocateStatement->set_expr_list (actualParameters);

  appendStatement (deallocateStatement, scope);
}

SgImplicitStatement *
FortranStatementsAndExpressionsBuilder::buildImplicitNoneStatement ()
{
  SgImplicitStatement * implicitStatement = new SgImplicitStatement (
      RoseHelper::getFileInfo (), true);

  implicitStatement->set_endOfConstruct (RoseHelper::getFileInfo ());
  implicitStatement->set_definingDeclaration (implicitStatement);

  return implicitStatement;
}

SgExprStatement *
FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
    SgScopeStatement * scope, SgExpression * parameter1,
    SgExpression * parameter2, SgExpression * parameter3)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine ("c_f_pointer", scope);

  SgExprListExp * actualParameters = buildExprListExp (parameter1, parameter2,
      parameter3);

  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  return buildExprStatement (subroutineCall);
}
