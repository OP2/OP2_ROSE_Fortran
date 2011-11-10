#include <FortranTypesBuilder.h>
#include <RoseHelper.h>
#include <rose.h>

/*
 * ======================================================
 * Allocation of class-wide static variables
 * ======================================================
 */

SgTypeInt * FortranTypesBuilder::twoByteIntegerType;
SgTypeInt * FortranTypesBuilder::fourByteIntegerType;
SgTypeFloat * FortranTypesBuilder::singlePrecisionFloatType;
SgTypeFloat * FortranTypesBuilder::doublePrecisionFloatType;

SgFunctionSymbol *
FortranTypesBuilder::buildFunctionDeclaration (
    std::string const & functionName, SgScopeStatement * scope)
{
  using namespace SageBuilder;

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

SgTypeInt *
FortranTypesBuilder::getTwoByteInteger ()
{
  if (twoByteIntegerType == NULL)
  {
    twoByteIntegerType = new SgTypeInt ();
    SgIntVal * bytesPerInt = new SgIntVal (RoseHelper::getFileInfo (), 2);
    bytesPerInt->set_endOfConstruct (RoseHelper::getFileInfo ());
    twoByteIntegerType->set_type_kind (bytesPerInt);
  }
  return twoByteIntegerType;
}

SgTypeInt *
FortranTypesBuilder::getFourByteInteger ()
{
  if (fourByteIntegerType == NULL)
  {
    fourByteIntegerType = new SgTypeInt ();
    SgIntVal * bytesPerInt = new SgIntVal (RoseHelper::getFileInfo (), 4);
    bytesPerInt->set_endOfConstruct (RoseHelper::getFileInfo ());
    fourByteIntegerType->set_type_kind (bytesPerInt);
  }
  return fourByteIntegerType;
}

SgTypeFloat *
FortranTypesBuilder::getSinglePrecisionFloat ()
{
  if (singlePrecisionFloatType == NULL)
  {
    singlePrecisionFloatType = new SgTypeFloat ();
    SgIntVal * bytesPerFloat = new SgIntVal (RoseHelper::getFileInfo (), 4);
    bytesPerFloat->set_endOfConstruct (RoseHelper::getFileInfo ());
    singlePrecisionFloatType->set_type_kind (bytesPerFloat);
  }
  return singlePrecisionFloatType;
}

SgTypeFloat *
FortranTypesBuilder::getDoublePrecisionFloat ()
{
  if (doublePrecisionFloatType == NULL)
  {
    doublePrecisionFloatType = new SgTypeFloat ();
    SgIntVal * bytesPerFloat = new SgIntVal (RoseHelper::getFileInfo (), 8);
    bytesPerFloat->set_endOfConstruct (RoseHelper::getFileInfo ());
    doublePrecisionFloatType->set_type_kind (bytesPerFloat);
  }
  return doublePrecisionFloatType;
}

SgArrayType *
FortranTypesBuilder::getArray_RankOne (SgType * baseType, int lowerBound,
    int upperBound)
{
  using namespace SageBuilder;

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
  using namespace SageBuilder;

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
  using namespace SageBuilder;

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
  using namespace SageBuilder;

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
  using namespace SageBuilder;

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
  using namespace SageBuilder;

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

  nonDefiningClassDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
  classDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
  classDefinition->get_declaration ()->get_declarationModifier ().get_accessModifier ().setUndefined ();

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
