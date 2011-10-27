#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranParallelLoop.h>
#include <ScopedVariableDeclarations.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <Debug.h>
#include <boost/lexical_cast.hpp>
#include <rose.h>

void
FortranOpDatDimensionsDeclaration::addFields ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using boost::lexical_cast;
  using std::string;

  fieldDeclarations = new ScopedVariableDeclarations ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDirect (i) || parallelLoop->isIndirect (i))
    {
      Debug::getInstance ()->debugMessage (
          "Adding dimensions field for OP_DAT " + lexical_cast <string> (i),
          Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      string const & variableName = OP2::VariableNames::getOpDatDimensionName (
          i);

      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          moduleScope);

      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      typeStatement->get_definition ()->append_member (fieldDeclaration);

      fieldDeclarations->add (variableName, fieldDeclaration);
    }
  }
}

void
FortranOpDatDimensionsDeclaration::addTypeDeclaration ()
{
  using namespace SageInterface;

  typeStatement = RoseStatementsAndExpressionsBuilder::buildTypeDeclaration (
      parallelLoop->getUserSubroutineName () + "_opDatDimensions", moduleScope);

  typeStatement->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (typeStatement, moduleScope);
}

SgClassType *
FortranOpDatDimensionsDeclaration::getType ()
{
  return typeStatement->get_type ();
}

SgVarRefExp *
FortranOpDatDimensionsDeclaration::getOpDatDimensionField (
    unsigned int OP_DAT_ArgumentGroup)
{
  return fieldDeclarations->getReference (
      OP2::VariableNames::getOpDatDimensionName (OP_DAT_ArgumentGroup));
}

FortranOpDatDimensionsDeclaration::FortranOpDatDimensionsDeclaration (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  parallelLoop (parallelLoop), moduleScope (moduleScope)
{
  Debug::getInstance ()->debugMessage (
      "Creating OP_DAT dimensions type declaration", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  addTypeDeclaration ();

  addFields ();
}
