#include "FortranCUDAOpDatCardinalitiesDeclaration.h"
#include "FortranParallelLoop.h"
#include "FortranTypesBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "ScopedVariableDeclarations.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include <boost/lexical_cast.hpp>
#include <rose.h>

void
FortranCUDAOpDatCardinalitiesDeclaration::addFields ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isCardinalityDeclarationNeeded (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating cardinality field for OP_DAT "
                + lexical_cast <string> (i), Debug::HIGHEST_DEBUG_LEVEL,
            __FILE__, __LINE__);

        string const & variableName = getOpDatCardinalityName (i);

        SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
            variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
            moduleScope);

        fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

        deviceDatatypeStatement->get_definition ()->append_member (
            fieldDeclaration);

        fieldDeclarations->add (variableName, fieldDeclaration);
      }
    }
  }
}

SgClassType *
FortranCUDAOpDatCardinalitiesDeclaration::getType ()
{
  return deviceDatatypeStatement->get_type ();
}

ScopedVariableDeclarations *
FortranCUDAOpDatCardinalitiesDeclaration::getFieldDeclarations ()
{
  return fieldDeclarations;
}

FortranCUDAOpDatCardinalitiesDeclaration::FortranCUDAOpDatCardinalitiesDeclaration (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  parallelLoop (parallelLoop), moduleScope (moduleScope)
{
  using namespace SageInterface;

  fieldDeclarations = new ScopedVariableDeclarations ();

  deviceDatatypeStatement
      = RoseStatementsAndExpressionsBuilder::buildTypeDeclaration (
          parallelLoop->getUserSubroutineName () + "_opDatCardinalities",
          moduleScope);

  deviceDatatypeStatement->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (deviceDatatypeStatement, moduleScope);

  addFields ();
}
