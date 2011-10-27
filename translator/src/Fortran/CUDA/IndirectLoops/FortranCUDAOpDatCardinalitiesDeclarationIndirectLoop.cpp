#include <FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop.h>
#include <FortranParallelLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <ScopedVariableDeclarations.h>
#include <CommonNamespaces.h>
#include <rose.h>

void
FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop::addIndirectLoopFields ()
{
  using namespace SageBuilder;
  using std::string;
  using std::vector;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isDuplicateOpDat (i) == false)
      {

        string const variableName1 =
            OP2::VariableNames::getLocalToGlobalMappingSizeName (i);

        SgVariableDeclaration * fieldDeclaration1 = buildVariableDeclaration (
            variableName1, FortranTypesBuilder::getFourByteInteger (), NULL,
            moduleScope);

        fieldDeclaration1->get_declarationModifier ().get_accessModifier ().setUndefined ();

        deviceDatatypeStatement->get_definition ()->append_member (
            fieldDeclaration1);

        fieldDeclarations->add (variableName1, fieldDeclaration1);
      }

      string const variableName2 =
          OP2::VariableNames::getGlobalToLocalMappingSizeName (i);

      SgVariableDeclaration * fieldDeclaration2 = buildVariableDeclaration (
          variableName2, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration2->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration2);

      fieldDeclarations->add (variableName2, fieldDeclaration2);

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

FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop::FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranCUDAOpDatCardinalitiesDeclaration (parallelLoop, moduleScope)
{
  addIndirectLoopFields ();
}
