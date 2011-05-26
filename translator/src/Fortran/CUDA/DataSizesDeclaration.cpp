#include <boost/lexical_cast.hpp>
#include <DataSizesDeclaration.h>
#include <FortranStatementsAndExpressionsBuilder.h>

std::string
DataSizesDeclaration::get_OP_DAT_SizeFieldName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + IndirectAndDirectLoop::Fortran::VariableSuffixes::Size;
}

SgClassType *
DataSizesDeclaration::getType ()
{
  return deviceDatatypeStatement->get_type ();
}

SgVariableDeclaration *
DataSizesDeclaration::getFieldDeclaration (std::string fieldName)
{
  return fieldDeclarations[fieldName];
}

DataSizesDeclaration::DataSizesDeclaration (std::string const & subroutineName,
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
