#include <boost/lexical_cast.hpp>
#include <CPPOpenCLDataSizesDeclaration.h>
#include <RoseStatementsAndExpressionsBuilder.h>

SgClassType *
CPPOpenCLDataSizesDeclaration::getType ()
{
  return deviceDatatypeStatement->get_type ();
}

SubroutineVariableDeclarations *
CPPOpenCLDataSizesDeclaration::getFieldDeclarations ()
{
  return fieldDeclarations;
}

CPPOpenCLDataSizesDeclaration::CPPOpenCLDataSizesDeclaration (
    std::string const & subroutineName, CPPParallelLoop * parallelLoop,
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
