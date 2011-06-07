#include <boost/lexical_cast.hpp>
#include <FortranCUDADataSizesDeclaration.h>
#include <FortranStatementsAndExpressionsBuilder.h>


SgClassType *
FortranCUDADataSizesDeclaration::getType ()
{
  return deviceDatatypeStatement->get_type ();
}

SgVariableDeclaration *
FortranCUDADataSizesDeclaration::getFieldDeclaration (std::string fieldName)
{
  return fieldDeclarations[fieldName];
}

FortranCUDADataSizesDeclaration::FortranCUDADataSizesDeclaration (std::string const & subroutineName,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope)
{
  using SageInterface::appendStatement;

  this->parallelLoop = parallelLoop;

  this->moduleScope = moduleScope;

  deviceDatatypeStatement
      = FortranStatementsAndExpressionsBuilder::buildTypeDeclaration (
          subroutineName + "_variableSizes", moduleScope);

  deviceDatatypeStatement->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (deviceDatatypeStatement, moduleScope);
}
