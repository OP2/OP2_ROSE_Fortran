#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranOpDatDimensionsDeclaration::addFields ()
{
  using SageInterface::appendStatement;
  using SageBuilder::buildVariableDeclaration;
  using std::string;

  fieldDeclarations = new VariableDeclarations ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = VariableNames::getOpDatDimensionName (i);

    SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
        variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
        moduleScope);

    fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    typeStatement->get_definition ()->append_member (fieldDeclaration);

    fieldDeclarations->add (variableName, fieldDeclaration);
  }
}

void
FortranOpDatDimensionsDeclaration::addTypeDeclaration ()
{
  using SageInterface::appendStatement;

  typeStatement = FortranStatementsAndExpressionsBuilder::buildTypeDeclaration (
      subroutineName + "_opDatDimensions", moduleScope);

  typeStatement->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (typeStatement, moduleScope);
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */
SgClassType *
FortranOpDatDimensionsDeclaration::getType ()
{
  return typeStatement->get_type ();
}

SgVariableDeclaration *
FortranOpDatDimensionsDeclaration::getOpDatDimensionField (
    unsigned int OP_DAT_ArgumentGroup)
{
  return fieldDeclarations->get (VariableNames::getOpDatDimensionName (
      OP_DAT_ArgumentGroup));
}

FortranOpDatDimensionsDeclaration::FortranOpDatDimensionsDeclaration (
    std::string const & subroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  subroutineName (subroutineName), parallelLoop (parallelLoop), moduleScope (
      moduleScope)
{
  addTypeDeclaration ();

  addFields ();
}
