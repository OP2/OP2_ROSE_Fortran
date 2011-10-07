#include <CPPOpenCLDataSizesDeclaration.h>
#include <ScopedVariableDeclarations.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <rose.h>

SgClassType *
CPPOpenCLDataSizesDeclaration::getType ()
{
  return deviceDatatypeStatement->get_type ();
}

ScopedVariableDeclarations *
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

  fieldDeclarations = new ScopedVariableDeclarations ();

  deviceDatatypeStatement
      = RoseStatementsAndExpressionsBuilder::buildTypeDeclaration (
          subroutineName + "_variableSizes", moduleScope);

  deviceDatatypeStatement->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (deviceDatatypeStatement, moduleScope);
}
