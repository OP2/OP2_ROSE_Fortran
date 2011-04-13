#include <DeviceDataSizesDeclaration.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
DeviceDataSizesDeclaration::addFields (ParallelLoop & parallelLoop,
    SgScopeStatement * moduleScope)
{
  using SageBuilder::buildVariableDeclaration;
  using std::string;
  using std::vector;

  vector <string> fourByteIntegers;
  fourByteIntegers.push_back (DeviceDataSizesFields::pindSizesSize);
  fourByteIntegers.push_back (DeviceDataSizesFields::pindOffsSize);
  fourByteIntegers.push_back (DeviceDataSizesFields::pblkMapSize);
  fourByteIntegers.push_back (DeviceDataSizesFields::poffsetSize);
  fourByteIntegers.push_back (DeviceDataSizesFields::pnelemsSize);
  fourByteIntegers.push_back (DeviceDataSizesFields::pnthrcolSize);
  fourByteIntegers.push_back (DeviceDataSizesFields::pthrcolSize);

  for (vector <string>::iterator it = fourByteIntegers.begin (); it
      != fourByteIntegers.end (); ++it)
  {
    SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (*it,
        FortranTypesBuilder::getFourByteInteger (), NULL, moduleScope);
    fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    deviceDatatypeStatement->get_definition ()->append_member (fieldDeclaration);
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

DeviceDataSizesDeclaration::DeviceDataSizesDeclaration (
    ParallelLoop & parallelLoop, std::string const & subroutineName,
    SgScopeStatement * moduleScope)
{
  using SageInterface::appendStatement;

  deviceDatatypeStatement
      = FortranStatementsAndExpressionsBuilder::buildTypeDeclaration (
          subroutineName + "_variableSizes", moduleScope);

  deviceDatatypeStatement->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (deviceDatatypeStatement, moduleScope);

  addFields (parallelLoop, moduleScope);
}
