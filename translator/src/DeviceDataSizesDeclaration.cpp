#include <boost/lexical_cast.hpp>
#include <DeviceDataSizesDeclaration.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <OP2CommonDefinitions.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
DeviceDataSizesDeclaration::addFields (ParallelLoop & parallelLoop,
    SgScopeStatement * moduleScope)
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using std::map;
  using std::string;
  using std::vector;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      string const variableName = kernelDatArgumentsNames::argNamePrefix + lexical_cast <string> (i)
          + kernelDatArgumentsNames::argNameSizePostfix;

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      OP_DAT_Sizes[i] = fieldDeclaration;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "pindMaps" + lexical_cast <string> (i)
          + "Size";

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      localToGlobalRenumberingOfIndirectMappingSizes[i] = fieldDeclaration;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "pmaps" + lexical_cast <string> (i) + "Size";

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      globalToLocalRenumberingOfIndirectMappingSizes[i] = fieldDeclaration;
    }
  }

  vector <string> planFunctionSizeVariables;
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pblkMapSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pindMapsSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pindOffsSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pindSizesSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pnelemsSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pnthrcolSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::poffsetSize);
  planFunctionSizeVariables.push_back (PlanFunctionSizeVariables::pthrcolSize);

  for (vector <string>::iterator it = planFunctionSizeVariables.begin (); it
      != planFunctionSizeVariables.end (); ++it)
  {
    SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (*it,
        FortranTypesBuilder::getFourByteInteger (), NULL, moduleScope);

    fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    deviceDatatypeStatement->get_definition ()->append_member (fieldDeclaration);

    otherFieldDeclarations[*it] = fieldDeclaration;
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

  // deviceDatatypeStatement->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (deviceDatatypeStatement, moduleScope);

 // addFields (parallelLoop, moduleScope);
}
