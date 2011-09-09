#include <FortranCUDADataSizesDeclarationIndirectLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <CommonNamespaces.h>
#include <Plan.h>

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
FortranCUDADataSizesDeclarationIndirectLoop::addFields ()
{
  using SageBuilder::buildVariableDeclaration;
  using std::string;
  using std::vector;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName = OP2::VariableNames::getOpDatSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      fieldDeclarations->add (variableName, fieldDeclaration);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          OP2::VariableNames::getLocalToGlobalMappingSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      fieldDeclarations->add (variableName, fieldDeclaration);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          OP2::VariableNames::getGlobalToLocalMappingSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      fieldDeclarations->add (variableName, fieldDeclaration);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && (parallelLoop->getOpMapValue (i) == DIRECT
            || parallelLoop->getOpMapValue (i) == GLOBAL))
    {
      string const variableName = OP2::VariableNames::getOpDatSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      fieldDeclarations->add (variableName, fieldDeclaration);
    }
  }

  vector <string> planFunctionSizeVariables;

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pblkMapSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pindOffsSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pindSizesSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pnelemsSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pnthrcolSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::poffsetSize);

  planFunctionSizeVariables.push_back (
      OP2::VariableNames::PlanFunction::pthrcolSize);

  for (vector <string>::iterator it = planFunctionSizeVariables.begin (); it
      != planFunctionSizeVariables.end (); ++it)
  {
    SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (*it,
        FortranTypesBuilder::getFourByteInteger (), NULL, moduleScope);

    fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    deviceDatatypeStatement->get_definition ()->append_member (fieldDeclaration);

    fieldDeclarations->add (*it, fieldDeclaration);
  }
}

FortranCUDADataSizesDeclarationIndirectLoop::FortranCUDADataSizesDeclarationIndirectLoop (
    std::string const & subroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranCUDADataSizesDeclaration (subroutineName, parallelLoop, moduleScope)
{
  addFields ();
}
