#include <boost/lexical_cast.hpp>
#include <FortranCUDADataSizesDeclaration.h>
#include <RoseStatementsAndExpressionsBuilder.h>

SgClassType *
FortranCUDADataSizesDeclaration::getType ()
{
  return deviceDatatypeStatement->get_type ();
}

SubroutineVariableDeclarations *
FortranCUDADataSizesDeclaration::getFieldDeclarations ()
{
  return fieldDeclarations;
}

FortranCUDADataSizesDeclaration::FortranCUDADataSizesDeclaration (
    std::string const & subroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  parallelLoop (parallelLoop), moduleScope (moduleScope)
{
  using SageInterface::appendStatement;

  fieldDeclarations = new SubroutineVariableDeclarations ();

  deviceDatatypeStatement
      = RoseStatementsAndExpressionsBuilder::buildTypeDeclaration (
          subroutineName + "_variableSizes", moduleScope);

  deviceDatatypeStatement->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (deviceDatatypeStatement, moduleScope);
}
