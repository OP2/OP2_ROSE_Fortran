#include "FortranTypeDeclarations.h"

/*
 * ======================================================
 * Allocation of class-wide static variables
 * ======================================================
 */

SgTypeInt * FortranTypeDeclarations::twoByteInteger;
SgTypeInt * FortranTypeDeclarations::fourByteInteger;
SgTypeInt * FortranTypeDeclarations::eightByteInteger;
SgTypeFloat * FortranTypeDeclarations::singlePrecisionFloat;
SgTypeFloat * FortranTypeDeclarations::doublePrecisionFloat;

Sg_File_Info * FortranTypeDeclarations::fileInfo =
    Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode ();

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgArrayType *
FortranTypeDeclarations::getArray_RankOne (int lowerBound, int upperBound,
    SgType * baseType)
{
  using SageBuilder::buildExprListExp;

  SgExpression * lowerBoundExpression;
  SgExpression * upperBoundExpression;

  if (lowerBound == -1)
  {
    lowerBoundExpression = new SgNullExpression (fileInfo);
  }
  else
  {
    lowerBoundExpression = new SgIntVal (fileInfo, lowerBound);
  }
  lowerBoundExpression->set_endOfConstruct (fileInfo);

  if (upperBound == -1)
  {
    upperBoundExpression = new SgNullExpression (fileInfo);
  }
  else
  {
    upperBoundExpression = new SgIntVal (fileInfo, upperBound);
  }
  upperBoundExpression->set_endOfConstruct (fileInfo);

  SgIntVal * strideExpression = new SgIntVal (fileInfo, 1);
  strideExpression->set_endOfConstruct (fileInfo);

  SgSubscriptExpression * arrayDimensionExpression = new SgSubscriptExpression (
      fileInfo, lowerBoundExpression, upperBoundExpression, strideExpression);
  arrayDimensionExpression->set_endOfConstruct (fileInfo);

  SgArrayType * array = new SgArrayType (baseType, arrayDimensionExpression);
  arrayDimensionExpression->set_parent (baseType);

  array->set_dim_info (buildExprListExp (arrayDimensionExpression));
  array->set_rank (1);

  return array;
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgTypeInt *
FortranTypeDeclarations::getTwoByteInteger ()
{
  if (twoByteInteger == NULL)
  {
    twoByteInteger = new SgTypeInt ();
    SgIntVal * bytesPerInt = new SgIntVal (fileInfo, 2);
    bytesPerInt->set_endOfConstruct (fileInfo);
    twoByteInteger->set_type_kind (bytesPerInt);
  }
  return twoByteInteger;
}

SgTypeInt *
FortranTypeDeclarations::getFourByteInteger ()
{
  if (fourByteInteger == NULL)
  {
    fourByteInteger = new SgTypeInt ();
    SgIntVal * bytesPerInt = new SgIntVal (fileInfo, 4);
    bytesPerInt->set_endOfConstruct (fileInfo);
    fourByteInteger->set_type_kind (bytesPerInt);
  }
  return fourByteInteger;
}

SgTypeInt *
FortranTypeDeclarations::getEightByteInteger ()
{
  if (eightByteInteger == NULL)
  {
    eightByteInteger = new SgTypeInt ();
    SgIntVal * bytesPerInt = new SgIntVal (fileInfo, 8);
    bytesPerInt->set_endOfConstruct (fileInfo);
    eightByteInteger->set_type_kind (bytesPerInt);
  }
  return eightByteInteger;
}

SgTypeFloat *
FortranTypeDeclarations::getSinglePrecisionFloat ()
{
  if (singlePrecisionFloat == NULL)
  {
    singlePrecisionFloat = new SgTypeFloat ();
    SgIntVal * bytesPerFloat = new SgIntVal (fileInfo, 4);
    bytesPerFloat->set_endOfConstruct (fileInfo);
    singlePrecisionFloat->set_type_kind (bytesPerFloat);
  }
  return singlePrecisionFloat;
}

SgTypeFloat *
FortranTypeDeclarations::getDoublePrecisionFloat ()
{
  if (doublePrecisionFloat == NULL)
  {
    doublePrecisionFloat = new SgTypeFloat ();
    SgIntVal * bytesPerFloat = new SgIntVal (fileInfo, 8);
    bytesPerFloat->set_endOfConstruct (fileInfo);
    doublePrecisionFloat->set_type_kind (bytesPerFloat);
  }
  return doublePrecisionFloat;
}

SgArrayType *
FortranTypeDeclarations::getTwoByteIntegerArray_RankOne (int lowerBound,
    int upperBound)
{
  return getArray_RankOne (lowerBound, upperBound, getTwoByteInteger ());
}

SgArrayType *
FortranTypeDeclarations::getFourByteIntegerArray_RankOne (int lowerBound,
    int upperBound)
{
  return getArray_RankOne (lowerBound, upperBound, getFourByteInteger ());
}

SgArrayType *
FortranTypeDeclarations::getEightByteIntegerArray_RankOne (int lowerBound,
    int upperBound)
{
  return getArray_RankOne (lowerBound, upperBound, getEightByteInteger ());
}

SgArrayType *
FortranTypeDeclarations::getSinglePrecisionFloatArray_RankOne (int lowerBound,
    int upperBound)
{
  return getArray_RankOne (lowerBound, upperBound, getSinglePrecisionFloat ());
}

SgArrayType *
FortranTypeDeclarations::getDoublePrecisionFloatArray_RankOne (int lowerBound,
    int upperBound)
{
  return getArray_RankOne (lowerBound, upperBound, getDoublePrecisionFloat ());
}

SgTypeString *
FortranTypeDeclarations::getString (int length)
{
  SgIntVal * numberOfCharacters = new SgIntVal (fileInfo, length);
  numberOfCharacters->set_endOfConstruct (fileInfo);
  SgTypeString * string = new SgTypeString (numberOfCharacters);
  return string;
}
