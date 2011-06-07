#include <FortranCUDADataSizesDeclarationDirectLoop.h>
#include <FortranTypesBuilder.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranCUDADataSizesDeclarationDirectLoop::addFields ()
{
  using SageBuilder::buildVariableDeclaration;
  using std::string;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      string const & variableName = VariableNames::getOpDatSizeName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      fieldDeclarations[variableName] = fieldDeclaration;
    }
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranCUDADataSizesDeclarationDirectLoop::FortranCUDADataSizesDeclarationDirectLoop (
    std::string const & subroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranCUDADataSizesDeclaration (subroutineName, parallelLoop, moduleScope)
{
  addFields ();
}
