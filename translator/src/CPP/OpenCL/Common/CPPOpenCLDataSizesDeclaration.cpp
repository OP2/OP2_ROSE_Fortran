#include <CPPOpenCLDataSizesDeclaration.h>
#include <SubroutineVariableDeclarations.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <rose.h>

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
