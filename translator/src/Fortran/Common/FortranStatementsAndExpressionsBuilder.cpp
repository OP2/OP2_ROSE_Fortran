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
  /*
   * ======================================================
   * The variable declaration is always undefined
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
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;

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
FortranStatementsAndExpressionsBuilder::getFortranKindOfOpDat (
    SgType * OpDatBaseType)
{
  using SageBuilder::buildIntVal;

  SgArrayType * isArrayType = isSgArrayType (OpDatBaseType);

  ROSE_ASSERT (isArrayType != NULL);

  SgType * arrayBaseType = isArrayType->get_base_type ();

  SgExpression * sizeOfOpDatKind = arrayBaseType->get_type_kind ();

  /*
   * ======================================================
   * If a Fortran kind has not been specified, then we
   * have to assume standard ones: integer(4) and real(4)
   * ======================================================
   */
  if (sizeOfOpDatKind == NULL)
  {
    sizeOfOpDatKind = buildIntVal (4);
  }

  return sizeOfOpDatKind;
}

SgImplicitStatement *
FortranStatementsAndExpressionsBuilder::buildImplicitNoneStatement ()
{
  SgImplicitStatement * implicitStatement = new SgImplicitStatement (
      ROSEHelper::getFileInfo (), true);

  implicitStatement->set_endOfConstruct (ROSEHelper::getFileInfo ());
  implicitStatement->set_definingDeclaration (implicitStatement);

  return implicitStatement;
}