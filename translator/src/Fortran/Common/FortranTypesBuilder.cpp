#include <FortranTypesBuilder.h>
#include <RoseHelper.h>

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
  classDefinition->set_endOfConstruct (RoseHelper::getFileInfo ());
  classDefinition->set_declaration (classDeclaration);

  nonDefiningClassDeclaration->set_endOfConstruct (RoseHelper::getFileInfo ());
  nonDefiningClassDeclaration->set_definingDeclaration (classDeclaration);
  nonDefiningClassDeclaration->set_firstNondefiningDeclaration (
      nonDefiningClassDeclaration);
  nonDefiningClassDeclaration->set_scope (scope);
  nonDefiningClassDeclaration->set_parent (scope);
  nonDefiningClassDeclaration->set_type (SgClassType::createType (
      nonDefiningClassDeclaration));
  nonDefiningClassDeclaration->setForward ();

  classDeclaration->set_endOfConstruct (RoseHelper::getFileInfo ());
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
    SgIntVal * bytesPerInt = new SgIntVal (RoseHelper::getFileInfo (), 2);
    bytesPerInt->set_endOfConstruct (RoseHelper::getFileInfo ());
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
    SgIntVal * bytesPerInt = new SgIntVal (RoseHelper::getFileInfo (), 4);
    bytesPerInt->set_endOfConstruct (RoseHelper::getFileInfo ());
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
    SgIntVal * bytesPerInt = new SgIntVal (RoseHelper::getFileInfo (), 8);
    bytesPerInt->set_endOfConstruct (RoseHelper::getFileInfo ());
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
    SgIntVal * bytesPerFloat = new SgIntVal (RoseHelper::getFileInfo (), 4);
    bytesPerFloat->set_endOfConstruct (RoseHelper::getFileInfo ());
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
    SgIntVal * bytesPerFloat = new SgIntVal (RoseHelper::getFileInfo (), 8);
    bytesPerFloat->set_endOfConstruct (RoseHelper::getFileInfo ());
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
    lowerBoundExpression = new SgNullExpression (RoseHelper::getFileInfo ());
  }
  else
  {
    lowerBoundExpression
        = new SgIntVal (RoseHelper::getFileInfo (), lowerBound);
  }
  lowerBoundExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  if (upperBound == -1)
  {
    upperBoundExpression = new SgNullExpression (RoseHelper::getFileInfo ());
  }
  else
  {
    upperBoundExpression
        = new SgIntVal (RoseHelper::getFileInfo (), upperBound);
  }
  upperBoundExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgIntVal * strideExpression = new SgIntVal (RoseHelper::getFileInfo (), 1);
  strideExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgSubscriptExpression * arrayDimensionExpression = new SgSubscriptExpression (
      RoseHelper::getFileInfo (), lowerBoundExpression, upperBoundExpression,
      strideExpression);
  arrayDimensionExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgArrayType * array = new SgArrayType (baseType, arrayDimensionExpression);
  arrayDimensionExpression->set_parent (baseType);

  array->set_dim_info (buildExprListExp (arrayDimensionExpression));
  array->set_rank (1);

  return array;
}

SgArrayType *
FortranTypesBuilder::getArrayTypePlainDimension (SgType * baseType,
    int dimension)
{
  using SageBuilder::buildArrayType;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildExprListExp;

  SgArrayType * returnedType = buildArrayType (baseType);
  SgExpression * dimensionExpression = buildIntVal (dimension);
  returnedType->set_dim_info (buildExprListExp (dimensionExpression));
  returnedType->set_rank (1);

  return returnedType;
}

SgArrayType *
FortranTypesBuilder::getArray_RankOne (SgType * baseType, int lowerBound,
    SgExpression * upperBoundExpression)
{
  using SageBuilder::buildExprListExp;

  SgExpression * lowerBoundExpression;

  if (lowerBound == -1)
  {
    lowerBoundExpression = new SgNullExpression (RoseHelper::getFileInfo ());
  }
  else
  {
    lowerBoundExpression
        = new SgIntVal (RoseHelper::getFileInfo (), lowerBound);
  }
  lowerBoundExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  if (upperBoundExpression == NULL)
  {
    upperBoundExpression = new SgNullExpression (RoseHelper::getFileInfo ());
  }

  upperBoundExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgIntVal * strideExpression = new SgIntVal (RoseHelper::getFileInfo (), 1);
  strideExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgSubscriptExpression * arrayDimensionExpression = new SgSubscriptExpression (
      RoseHelper::getFileInfo (), lowerBoundExpression, upperBoundExpression,
      strideExpression);
  arrayDimensionExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

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

  lowerBoundExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgExpression * upperBoundExpression = new SgNullExpression ();
  upperBoundExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgIntVal * strideExpression = new SgIntVal (RoseHelper::getFileInfo (), 1);
  strideExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgSubscriptExpression * arrayDimensionExpression = new SgSubscriptExpression (
      RoseHelper::getFileInfo (), lowerBoundExpression, upperBoundExpression,
      strideExpression);
  arrayDimensionExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

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

  upperBoundExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgExpression * lowerBoundExpression = new SgNullExpression ();
  lowerBoundExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgIntVal * strideExpression = new SgIntVal (RoseHelper::getFileInfo (), 1);
  strideExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgSubscriptExpression * arrayDimensionExpression = new SgSubscriptExpression (
      RoseHelper::getFileInfo (), lowerBoundExpression, upperBoundExpression,
      strideExpression);
  arrayDimensionExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

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

  lowerBoundExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  upperBoundExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgIntVal * strideExpression = new SgIntVal (RoseHelper::getFileInfo (), 1);
  strideExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgSubscriptExpression * arrayDimensionExpression = new SgSubscriptExpression (
      RoseHelper::getFileInfo (), lowerBoundExpression, upperBoundExpression,
      strideExpression);
  arrayDimensionExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgArrayType * array = new SgArrayType (baseType, arrayDimensionExpression);
  arrayDimensionExpression->set_parent (baseType);

  array->set_dim_info (buildExprListExp (arrayDimensionExpression));
  array->set_rank (1);

  return array;
}

SgTypeString *
FortranTypesBuilder::getString (int length)
{
  SgIntVal * numberOfCharacters = new SgIntVal (RoseHelper::getFileInfo (),
      length);
  numberOfCharacters->set_endOfConstruct (RoseHelper::getFileInfo ());
  SgTypeString * string = new SgTypeString (numberOfCharacters);
  return string;
}

SgModuleStatement *
FortranTypesBuilder::buildModuleDeclaration (std::string const & moduleName,
    SgScopeStatement * scope)
{
  SgClassDefinition * classDefinition = new SgClassDefinition (
      RoseHelper::getFileInfo ());

  SgModuleStatement * classDeclaration = new SgModuleStatement (
      RoseHelper::getFileInfo (), moduleName, SgClassDeclaration::e_class,
      NULL, classDefinition);

  SgClassDeclaration * nonDefiningClassDeclaration = new SgClassDeclaration (
      RoseHelper::getFileInfo (), moduleName, SgClassDeclaration::e_class,
      NULL, NULL);

  completeNewDeclaration (classDefinition, classDeclaration,
      nonDefiningClassDeclaration, scope);

  return classDeclaration;
}

SgClassDeclaration *
FortranTypesBuilder::buildClassDeclaration (std::string const & typeName,
    SgScopeStatement * scope)
{
  SgClassDefinition * classDefinition = new SgClassDefinition (
      RoseHelper::getFileInfo ());

  SgClassDeclaration * classDeclaration = new SgClassDeclaration (
      RoseHelper::getFileInfo (), typeName, SgClassDeclaration::e_class, NULL,
      classDefinition);

  SgClassDeclaration * nonDefiningClassDeclaration = new SgClassDeclaration (
      RoseHelper::getFileInfo (), typeName, SgClassDeclaration::e_class, NULL,
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
