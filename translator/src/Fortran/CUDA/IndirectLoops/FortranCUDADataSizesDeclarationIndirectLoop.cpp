#include <FortranCUDADataSizesDeclarationIndirectLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <CommonNamespaces.h>

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

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName = VariableNames::getOpDatSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      fieldDeclarations[variableName] = fieldDeclaration;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          VariableNames::getLocalToGlobalMappingSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      fieldDeclarations[variableName] = fieldDeclaration;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          VariableNames::getGlobalToLocalMappingSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      fieldDeclarations[variableName] = fieldDeclaration;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == DIRECT)
    {
      string const variableName = VariableNames::getOpDatSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      fieldDeclarations[variableName] = fieldDeclaration;
    }
  }

  vector <string> planFunctionSizeVariables;

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pblkMapSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindOffsSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pindSizesSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnelemsSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pnthrcolSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::poffsetSize);

  planFunctionSizeVariables.push_back (
      IndirectLoop::Fortran::PlanFunction::VariableNames::pthrcolSize);

  for (vector <string>::iterator it = planFunctionSizeVariables.begin (); it
      != planFunctionSizeVariables.end (); ++it)
  {
    SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (*it,
        FortranTypesBuilder::getFourByteInteger (), NULL, moduleScope);

    fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    deviceDatatypeStatement->get_definition ()->append_member (fieldDeclaration);

    fieldDeclarations[*it] = fieldDeclaration;
  }
}

FortranCUDADataSizesDeclarationIndirectLoop::FortranCUDADataSizesDeclarationIndirectLoop (
    std::string const & subroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranCUDADataSizesDeclaration (subroutineName, parallelLoop, moduleScope)
{
  addFields ();
}
