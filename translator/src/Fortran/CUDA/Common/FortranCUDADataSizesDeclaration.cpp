#include <boost/lexical_cast.hpp>
#include <FortranCUDADataSizesDeclaration.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

void
FortranCUDADataSizesDeclaration::addDataSizesFields ()
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      /*
       * ======================================================
       * The only time we do NOT need a sizes field is when:
       * a) The data is OP_GBL
       * b) The data is scalar
       * c) The data is read
       *
       * This is because, in this case, there is no transferal of
       * OP_GBL data into device memory. The data is instead
       * passed by value directly to the CUDA kernel.
       * ======================================================
       */

      bool sizeFieldNeeded = parallelLoop->isGlobal (i) == false
          || parallelLoop->isGlobalNonScalar (i)
          || (parallelLoop->isGlobalScalar (i) && parallelLoop->isRead (i)
              == false);

      if (sizeFieldNeeded)
      {
        Debug::getInstance ()->debugMessage ("Creating size field for OP_DAT "
            + lexical_cast <string> (i), Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
            __LINE__);

        string const & variableName = OP2::VariableNames::getOpDatSizeName (i);

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

  addDataSizesFields ();
}
