#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <CommonNamespaces.h>
#include <ROSEHelper.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranStatementsAndExpressionsBuilder::setFortranAttributes (
    SgVariableDeclaration * variableDeclaration, int remainingArguments,
    va_list attributeArguments)
{
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

      case CONSTANT:
      {
        variableDeclaration->get_declarationModifier ().get_typeModifier ().setConstant ();
        break;
      }

      case DEVICE:
      {
        variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
        break;
      }

      case SHARED:
      {
        variableDeclaration->get_declarationModifier ().get_typeModifier ().setShared ();
        break;
      }

      case VALUE:
      {
        variableDeclaration->get_declarationModifier ().get_typeModifier ().setValue ();
        break;
      }

      default:
      {
        Debug::getInstance ()->errorMessage ("Unhandled Fortran attribute");
      }
    }
  }
}

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

SgDerivedTypeStatement *
FortranStatementsAndExpressionsBuilder::buildTypeDeclaration (
    std::string const & typeName, SgScopeStatement * scope)
{
  SgClassDefinition * classDefinition = new SgClassDefinition (
      ROSEHelper::getFileInfo ());
  classDefinition->set_endOfConstruct (ROSEHelper::getFileInfo ());
  classDefinition->setCaseInsensitive (true);

  SgDerivedTypeStatement* classDeclaration = new SgDerivedTypeStatement (
      ROSEHelper::getFileInfo (), typeName, SgClassDeclaration::e_struct, NULL,
      classDefinition);
  classDeclaration->set_endOfConstruct (ROSEHelper::getFileInfo ());
  classDeclaration->set_definingDeclaration (classDeclaration);

  SgDerivedTypeStatement* nondefiningClassDeclaration =
      new SgDerivedTypeStatement (ROSEHelper::getFileInfo (), typeName,
          SgClassDeclaration::e_struct, NULL, NULL);
  nondefiningClassDeclaration->set_endOfConstruct (ROSEHelper::getFileInfo ());
  nondefiningClassDeclaration->set_parent (scope);

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

  classDeclaration->set_scope (scope);
  nondefiningClassDeclaration->set_scope (scope);

  classDeclaration->set_parent (scope);

  SgClassSymbol * classSymbol = new SgClassSymbol (nondefiningClassDeclaration);

  scope->insert_symbol (typeName, classSymbol);

  return classDeclaration;
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
    std::string const & variableName, SgType * type, SgScopeStatement * scope,
    int remainingArguments, ...)
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, type, NULL, scope);

  appendStatement (variableDeclaration, scope);

  variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  va_list fortranAttributes;

  va_start (fortranAttributes, remainingArguments);

  setFortranAttributes (variableDeclaration, remainingArguments,
      fortranAttributes);

  va_end (fortranAttributes);

  return variableDeclaration;
}

void
FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
    std::string const & variableName, SgType * type, SgScopeStatement * scope,
    SgFunctionParameterList * formalParameters, int remainingArguments, ...)
{
  SgVariableDeclaration * variableDeclaration = appendVariableDeclaration (
      variableName, type, scope);

  va_list fortranAttributes;

  va_start (fortranAttributes, remainingArguments);

  setFortranAttributes (variableDeclaration, remainingArguments,
      fortranAttributes);

  va_end (fortranAttributes);
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

SgExpression *
FortranStatementsAndExpressionsBuilder::getFortranKindOf_OP_DAT (
    SgType * OP_DAT_baseType)
{
  using SageBuilder::buildIntVal;

  SgArrayType * isArrayType = isSgArrayType (OP_DAT_baseType);

  ROSE_ASSERT ( isArrayType != NULL );

  SgType * arrayBaseType = isArrayType->get_base_type ();

  SgExpression * sizeOf_OP_DAT_Kind = arrayBaseType->get_type_kind ();

  /*
   * ======================================================
   * If a Fortran kind has not been specified, then we
   * have to assume standard ones: integer(4) and real(4)
   * ======================================================
   */
  if (sizeOf_OP_DAT_Kind == NULL)
  {
    if (isSgTypeInt (arrayBaseType) != NULL)
    {
      sizeOf_OP_DAT_Kind = buildIntVal (
          FortranVariableDeafultKinds::DEFAULT_KIND_INT);
    }
    else
    {
      sizeOf_OP_DAT_Kind = buildIntVal (
          FortranVariableDeafultKinds::DEFAULT_KIND_REAL);
    }
  }

  return sizeOf_OP_DAT_Kind;
}
