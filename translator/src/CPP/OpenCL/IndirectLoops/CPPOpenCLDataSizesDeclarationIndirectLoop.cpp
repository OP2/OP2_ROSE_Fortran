#include <CPPOpenCLDataSizesDeclarationIndirectLoop.h>
#include <CPPStatementsAndExpressionsBuilder.h>
#include <CPPTypesBuilder.h>
#include <CommonNamespaces.h>
#include <Plan.h>

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
CPPOpenCLDataSizesDeclarationIndirectLoop::addFields ()
{
  using SageBuilder::buildVariableDeclaration;
  using std::string;
  using std::vector;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName = VariableNames::getOpDatSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, CPPTypesBuilder::getFourByteInteger (), NULL,
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
          VariableNames::getLocalToGlobalMappingSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, CPPTypesBuilder::getFourByteInteger (), NULL,
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
          VariableNames::getGlobalToLocalMappingSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, CPPTypesBuilder::getFourByteInteger (), NULL,
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
      string const variableName = VariableNames::getOpDatSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, CPPTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      fieldDeclarations->add (variableName, fieldDeclaration);
    }
  }

  vector <string> planFunctionSizeVariables;

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pblkMapSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pindOffsSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pindSizesSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pnelemsSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pnthrcolSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::poffsetSize);

  planFunctionSizeVariables.push_back (PlanFunction::CPP::pthrcolSize);

  for (vector <string>::iterator it = planFunctionSizeVariables.begin (); it
      != planFunctionSizeVariables.end (); ++it)
  {
    SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (*it,
        CPPTypesBuilder::getFourByteInteger (), NULL, moduleScope);

    fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    deviceDatatypeStatement->get_definition ()->append_member (fieldDeclaration);

    fieldDeclarations->add (*it, fieldDeclaration);
  }
}

CPPOpenCLDataSizesDeclarationIndirectLoop::CPPOpenCLDataSizesDeclarationIndirectLoop (
    std::string const & subroutineName, CPPParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  CPPOpenCLDataSizesDeclaration (subroutineName, parallelLoop, moduleScope)
{
  addFields ();
}
