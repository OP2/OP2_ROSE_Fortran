#include <boost/lexical_cast.hpp>
#include <FortranCUDADataSizesDeclaration.h>
#include <FortranStatementsAndExpressionsBuilder.h>

SgClassType *
FortranCUDADataSizesDeclaration::getType ()
{
  return deviceDatatypeStatement->get_type ();
}

VariableDeclarations *
FortranCUDADataSizesDeclaration::getFieldDeclarations ()
{
  return fieldDeclarations;
}

FortranCUDADataSizesDeclaration::FortranCUDADataSizesDeclaration (
    std::string const & subroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  parallelLoop (parallelLoop), moduleScope (moduleScope)
{
  using SageInterface::appendStatement;

  fieldDeclarations = new VariableDeclarations ();

  deviceDatatypeStatement
      = FortranStatementsAndExpressionsBuilder::buildTypeDeclaration (
          subroutineName + "_variableSizes", moduleScope);

  deviceDatatypeStatement->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (deviceDatatypeStatement, moduleScope);
}
