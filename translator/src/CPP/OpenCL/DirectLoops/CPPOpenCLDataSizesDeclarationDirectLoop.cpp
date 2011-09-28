#include <CPPOpenCLDataSizesDeclarationDirectLoop.h>
//#include <CPPTypesBuilder.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPOpenCLDataSizesDeclarationDirectLoop::addFields ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = OP2::VariableNames::getOpDatCardinalityName (i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, buildIntType (), NULL, moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      deviceDatatypeStatement->get_definition ()->append_member (
          fieldDeclaration);

      fieldDeclarations->add (variableName, fieldDeclaration);
    }
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPOpenCLDataSizesDeclarationDirectLoop::CPPOpenCLDataSizesDeclarationDirectLoop (
    std::string const & subroutineName, CPPParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  CPPOpenCLDataSizesDeclaration (subroutineName, parallelLoop, moduleScope)
{
  addFields ();

}
