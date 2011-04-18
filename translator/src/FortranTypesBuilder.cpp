#include <FortranTypesBuilder.h>
#include <ROSEHelper.h>

/*
 * ======================================================
 * Allocation of class-wide static variables
 * ======================================================
 */

SgTypeInt * FortranTypesBuilder::twoByteInteger;
SgTypeInt * FortranTypesBuilder::fourByteInteger;
SgTypeInt * FortranTypesBuilder::eightByteInteger;
SgTypeFloat * FortranTypesBuilder::singlePrecisionFloat;
SgTypeFloat * FortranTypesBuilder::doublePrecisionFloat;

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgFunctionSymbol *
FortranTypesBuilder::buildFunctionDeclaration (
    std::string const & functionName, SgScopeStatement * scope)
{
  using SageBuilder::buildFunctionType;
  using SageBuilder::buildFunctionRefExp;
  using SageBuilder::buildVoidType;

  SgFunctionType * functionType = buildFunctionType (buildVoidType (),
      new SgFunctionParameterTypeList ());

  SgFunctionRefExp * functionReference = buildFunctionRefExp (functionName,
      functionType, scope);

  return functionReference->get_symbol ();
}

void
FortranTypesBuilder::completeNewDeclaration (
    SgClassDefinition * classDefinition, SgClassDeclaration * classDeclaration,
    SgClassDeclaration * nonDefiningClassDeclaration, SgScopeStatement * scope)
{
  classDefinition->set_endOfConstruct (ROSEHelper::getFileInfo ());
  classDefinition->set_declaration (classDeclaration);

  nonDefiningClassDeclaration->set_endOfConstruct (ROSEHelper::getFileInfo ());
  nonDefiningClassDeclaration->set_definingDeclaration (classDeclaration);
  nonDefiningClassDeclaration->set_firstNondefiningDeclaration (
      nonDefiningClassDeclaration);
  nonDefiningClassDeclaration->set_scope (scope);
  nonDefiningClassDeclaration->set_parent (scope);
  nonDefiningClassDeclaration->set_type (SgClassType::createType (
      nonDefiningClassDeclaration));
  nonDefiningClassDeclaration->setForward ();

  classDeclaration->set_endOfConstruct (ROSEHelper::getFileInfo ());
  classDeclaration->set_definingDeclaration (classDeclaration);
  classDeclaration->set_firstNondefiningDeclaration (
      nonDefiningClassDeclaration);
  classDeclaration->set_scope (scope);
  classDeclaration->set_parent (scope);
  classDeclaration->set_type (nonDefiningClassDeclaration->get_type ());

  SgClassSymbol * classSymbol = new SgClassSymbol (classDeclaration);
  scope->insert_symbol (classDeclaration->get_name (), classSymbol);
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgTypeInt *
FortranTypesBuilder::getTwoByteInteger ()
{
  if (twoByteInteger == NULL)
  {
    twoByteInteger = new SgTypeInt ();
    SgIntVal * bytesPerInt = new SgIntVal (ROSEHelper::getFileInfo (), 2);
    bytesPerInt->set_endOfConstruct (ROSEHelper::getFileInfo ());
    twoByteInteger->set_type_kind (bytesPerInt);
  }
  return twoByteInteger;
}

SgTypeInt *
FortranTypesBuilder::getFourByteInteger ()
{
  if (fourByteInteger == NULL)
  {
    fourByteInteger = new SgTypeInt ();
    SgIntVal * bytesPerInt = new SgIntVal (ROSEHelper::getFileInfo (), 4);
    bytesPerInt->set_endOfConstruct (ROSEHelper::getFileInfo ());
    fourByteInteger->set_type_kind (bytesPerInt);
  }
  return fourByteInteger;
}

SgTypeInt *
FortranTypesBuilder::getEightByteInteger ()
{
  if (eightByteInteger == NULL)
  {
    eightByteInteger = new SgTypeInt ();
    SgIntVal * bytesPerInt = new SgIntVal (ROSEHelper::getFileInfo (), 8);
    bytesPerInt->set_endOfConstruct (ROSEHelper::getFileInfo ());
    eightByteInteger->set_type_kind (bytesPerInt);
  }
  return eightByteInteger;
}

SgTypeFloat *
FortranTypesBuilder::getSinglePrecisionFloat ()
{
  if (singlePrecisionFloat == NULL)
  {
    singlePrecisionFloat = new SgTypeFloat ();
    SgIntVal * bytesPerFloat = new SgIntVal (ROSEHelper::getFileInfo (), 4);
    bytesPerFloat->set_endOfConstruct (ROSEHelper::getFileInfo ());
    singlePrecisionFloat->set_type_kind (bytesPerFloat);
  }
  return singlePrecisionFloat;
}

SgTypeFloat *
FortranTypesBuilder::getDoublePrecisionFloat ()
{
  if (doublePrecisionFloat == NULL)
  {
    doublePrecisionFloat = new SgTypeFloat ();
    SgIntVal * bytesPerFloat = new SgIntVal (ROSEHelper::getFileInfo (), 8);
    bytesPerFloat->set_endOfConstruct (ROSEHelper::getFileInfo ());
    doublePrecisionFloat->set_type_kind (bytesPerFloat);
  }
  return doublePrecisionFloat;
}

SgArrayType *
FortranTypesBuilder::getArray_RankOne (SgType * baseType, int lowerBound,
    int upperBound)
{
  using SageBuilder::buildExprListExp;

  SgExpression * lowerBoundExpression;
  SgExpression * upperBoundExpression;

  if (lowerBound == -1)
  {
    lowerBoundExpression = new SgNullExpression (ROSEHelper::getFileInfo ());
  }
  else
  {
    lowerBoundExpression
        = new SgIntVal (ROSEHelper::getFileInfo (), lowerBound);
  }
  lowerBoundExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  if (upperBound == -1)
  {
    upperBoundExpression = new SgNullExpression (ROSEHelper::getFileInfo ());
  }
  else
  {
    upperBoundExpression
        = new SgIntVal (ROSEHelper::getFileInfo (), upperBound);
  }
  upperBoundExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgIntVal * strideExpression = new SgIntVal (ROSEHelper::getFileInfo (), 1);
  strideExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgSubscriptExpression * arrayDimensionExpression = new SgSubscriptExpression (
      ROSEHelper::getFileInfo (), lowerBoundExpression, upperBoundExpression,
      strideExpression);
  arrayDimensionExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgArrayType * array = new SgArrayType (baseType, arrayDimensionExpression);
  arrayDimensionExpression->set_parent (baseType);

  array->set_dim_info (buildExprListExp (arrayDimensionExpression));
  array->set_rank (1);

  return array;
}

SgArrayType *
FortranTypesBuilder::getArray_RankOne_WithLowerBound (SgType * baseType,
    SgExpression * lowerBoundExpression)
{
  using SageBuilder::buildExprListExp;

  lowerBoundExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgExpression * upperBoundExpression = new SgNullExpression ();
  upperBoundExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgIntVal * strideExpression = new SgIntVal (ROSEHelper::getFileInfo (), 1);
  strideExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgSubscriptExpression * arrayDimensionExpression = new SgSubscriptExpression (
      ROSEHelper::getFileInfo (), lowerBoundExpression, upperBoundExpression,
      strideExpression);
  arrayDimensionExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgArrayType * array = new SgArrayType (baseType, arrayDimensionExpression);
  arrayDimensionExpression->set_parent (baseType);

  array->set_dim_info (buildExprListExp (arrayDimensionExpression));
  array->set_rank (1);

  return array;
}

SgArrayType *
FortranTypesBuilder::getArray_RankOne_WithUpperBound (SgType * baseType,
    SgExpression * upperBoundExpression)
{
  using SageBuilder::buildExprListExp;

  upperBoundExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgExpression * lowerBoundExpression = new SgNullExpression ();
  lowerBoundExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgIntVal * strideExpression = new SgIntVal (ROSEHelper::getFileInfo(), 1);
  strideExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgSubscriptExpression * arrayDimensionExpression = new SgSubscriptExpression (
      ROSEHelper::getFileInfo (), lowerBoundExpression, upperBoundExpression,
      strideExpression);
  arrayDimensionExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgArrayType * array = new SgArrayType (baseType, arrayDimensionExpression);
  arrayDimensionExpression->set_parent (baseType);

  array->set_dim_info (buildExprListExp (arrayDimensionExpression));
  array->set_rank (1);

  return array;
}

SgArrayType *
FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
    SgType * baseType, SgExpression * lowerBoundExpression,
    SgExpression * upperBoundExpression)
{
  using SageBuilder::buildExprListExp;

  lowerBoundExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  upperBoundExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgIntVal * strideExpression = new SgIntVal (ROSEHelper::getFileInfo(), 1);
  strideExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgSubscriptExpression * arrayDimensionExpression = new SgSubscriptExpression (
      ROSEHelper::getFileInfo (), lowerBoundExpression, upperBoundExpression,
      strideExpression);
  arrayDimensionExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

  SgArrayType * array = new SgArrayType (baseType, arrayDimensionExpression);
  arrayDimensionExpression->set_parent (baseType);

  array->set_dim_info (buildExprListExp (arrayDimensionExpression));
  array->set_rank (1);

  return array;
}

SgTypeString *
FortranTypesBuilder::getString (int length)
{
  SgIntVal * numberOfCharacters = new SgIntVal (ROSEHelper::getFileInfo (),
      length);
  numberOfCharacters->set_endOfConstruct (ROSEHelper::getFileInfo ());
  SgTypeString * string = new SgTypeString (numberOfCharacters);
  return string;
}

SgModuleStatement *
FortranTypesBuilder::buildNewFortranModuleDeclaration (
    std::string const & moduleName, SgScopeStatement * scope)
{
  SgClassDefinition * classDefinition = new SgClassDefinition (
      ROSEHelper::getFileInfo ());

  SgModuleStatement * classDeclaration = new SgModuleStatement (
      ROSEHelper::getFileInfo (), moduleName, SgClassDeclaration::e_class,
      NULL, classDefinition);

  SgClassDeclaration * nonDefiningClassDeclaration = new SgClassDeclaration (
      ROSEHelper::getFileInfo (), moduleName, SgClassDeclaration::e_class,
      NULL, NULL);

  completeNewDeclaration (classDefinition, classDeclaration,
      nonDefiningClassDeclaration, scope);

  return classDeclaration;
}

SgClassDeclaration *
FortranTypesBuilder::buildNewTypeDeclaration (std::string const & typeName,
    SgScopeStatement * scope)
{
  SgClassDefinition * classDefinition = new SgClassDefinition (
      ROSEHelper::getFileInfo ());

  SgClassDeclaration * classDeclaration = new SgClassDeclaration (
      ROSEHelper::getFileInfo (), typeName, SgClassDeclaration::e_class, NULL,
      classDefinition);

  SgClassDeclaration * nonDefiningClassDeclaration = new SgClassDeclaration (
      ROSEHelper::getFileInfo (), typeName, SgClassDeclaration::e_class, NULL,
      NULL);

  completeNewDeclaration (classDefinition, classDeclaration,
      nonDefiningClassDeclaration, scope);

  return classDeclaration;
}

SgFunctionSymbol *
FortranTypesBuilder::buildNewFortranFunction (std::string const & functionName,
    SgScopeStatement * scope)
{
  SgFunctionSymbol * functionSymbol = buildFunctionDeclaration (functionName,
      scope);

  SgProcedureHeaderStatement* procedureHeaderStatement =
      isSgProcedureHeaderStatement (functionSymbol->get_declaration ());

  /*
   * ======================================================
   * Ensure that the new declaration is of type function
   * ======================================================
   */
  procedureHeaderStatement->set_subprogram_kind (
      SgProcedureHeaderStatement::e_function_subprogram_kind);

  return functionSymbol;
}

SgFunctionSymbol *
FortranTypesBuilder::buildNewFortranSubroutine (
    std::string const & functionName, SgScopeStatement * scope)
{
  SgFunctionSymbol * functionSymbol = buildFunctionDeclaration (functionName,
      scope);

  SgProcedureHeaderStatement* procedureHeaderStatement =
      isSgProcedureHeaderStatement (functionSymbol->get_declaration ());

  /*
   * ======================================================
   * Ensure that the new declaration is of type subroutine
   * ======================================================
   */

  procedureHeaderStatement->set_subprogram_kind (
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind);

  return functionSymbol;
}
