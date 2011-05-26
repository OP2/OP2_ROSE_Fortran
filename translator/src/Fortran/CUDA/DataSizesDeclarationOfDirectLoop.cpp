#include <DataSizesDeclarationOfDirectLoop.h>
#include <FortranTypesBuilder.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
DataSizesDeclarationOfDirectLoop::addFields ()
{
  using SageBuilder::buildVariableDeclaration;
  using std::string;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      string const & variableName = get_OP_DAT_SizeFieldName (i);

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

DataSizesDeclarationOfDirectLoop::DataSizesDeclarationOfDirectLoop (
    std::string const & subroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  DataSizesDeclaration (subroutineName, parallelLoop, moduleScope)
{
  addFields ();
}
