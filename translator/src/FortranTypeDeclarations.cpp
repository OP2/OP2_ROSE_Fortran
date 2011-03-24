#include "FortranTypeDeclarations.h"

SgModifierType * FortranTypeDeclarations::twoByteInteger;
SgModifierType * FortranTypeDeclarations::fourByteInteger;
SgModifierType * FortranTypeDeclarations::eightByteInteger;
SgModifierType * FortranTypeDeclarations::singlePrecisionFloat;
SgModifierType * FortranTypeDeclarations::doublePrecisionFloat;

SgModifierType *
FortranTypeDeclarations::getTwoByteInteger ()
{
  using SageBuilder::buildFortranKindType;
  using SageBuilder::buildIntType;
  using SageBuilder::buildIntVal;

  if (twoByteInteger == NULL)
  {
    twoByteInteger = buildFortranKindType (buildIntType (), buildIntVal (2));
  }
  return twoByteInteger;
}

SgModifierType *
FortranTypeDeclarations::getFourByteInteger ()
{
  using SageBuilder::buildFortranKindType;
  using SageBuilder::buildIntType;
  using SageBuilder::buildIntVal;

  if (fourByteInteger == NULL)
  {
    fourByteInteger = buildFortranKindType (buildIntType (), buildIntVal (4));
  }
  return fourByteInteger;
}

SgModifierType *
FortranTypeDeclarations::getEightByteInteger ()
{
  using SageBuilder::buildFortranKindType;
  using SageBuilder::buildIntType;
  using SageBuilder::buildIntVal;

  if (eightByteInteger == NULL)
  {
    eightByteInteger = buildFortranKindType (buildIntType (), buildIntVal (8));
  }
  return eightByteInteger;
}

SgModifierType *
FortranTypeDeclarations::getSinglePrecisionFloat ()
{
  using SageBuilder::buildFortranKindType;
  using SageBuilder::buildFloatType;
  using SageBuilder::buildIntVal;

  if (singlePrecisionFloat == NULL)
  {
    singlePrecisionFloat = buildFortranKindType (buildFloatType (),
        buildIntVal (4));
  }
  return singlePrecisionFloat;
}

SgModifierType *
FortranTypeDeclarations::getDoublePrecisionFloat ()
{
  using SageBuilder::buildFortranKindType;
  using SageBuilder::buildFloatType;
  using SageBuilder::buildIntVal;

  if (doublePrecisionFloat == NULL)
  {
    doublePrecisionFloat = buildFortranKindType (buildFloatType (),
        buildIntVal (4));
  }
  return doublePrecisionFloat;
}
