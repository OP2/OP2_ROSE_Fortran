#include <CPPOpDatDimensionsDeclaration.h>
#include <CPPParallelLoop.h>
#include <ScopedVariableDeclarations.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <rose.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPOpDatDimensionsDeclaration::addFields ()
{
  using SageInterface::appendStatement;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using std::string;

  fieldDeclarations = new ScopedVariableDeclarations ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = OP2::VariableNames::getOpDatDimensionName (i);

    SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
        variableName, buildIntType (), NULL, moduleScope);

    fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    typeStatement->get_definition ()->append_member (fieldDeclaration);

    fieldDeclarations->add (variableName, fieldDeclaration);
  }
}

void
CPPOpDatDimensionsDeclaration::addTypeDeclaration ()
{
  using SageInterface::appendStatement;

  typeStatement = RoseStatementsAndExpressionsBuilder::buildTypeDeclaration (
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
CPPOpDatDimensionsDeclaration::getType ()
{
  return typeStatement->get_type ();
}

SgVariableDeclaration *
CPPOpDatDimensionsDeclaration::getOpDatDimensionField (
    unsigned int OP_DAT_ArgumentGroup)
{
  return fieldDeclarations->get (OP2::VariableNames::getOpDatDimensionName (
      OP_DAT_ArgumentGroup));
}

CPPOpDatDimensionsDeclaration::CPPOpDatDimensionsDeclaration (
    std::string const & subroutineName, CPPParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  subroutineName (subroutineName), parallelLoop (parallelLoop), moduleScope (
      moduleScope)
{
  addTypeDeclaration ();

  addFields ();
}
