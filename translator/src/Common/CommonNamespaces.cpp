#include <boost/lexical_cast.hpp>
#include <CommonNamespaces.h>
#include <FortranTypesBuilder.h>

std::string
TargetBackends::toString (BACKEND_VALUE backend)
{
  switch (backend)
  {
    case CUDA:
    {
      return "CUDA";
    }

    case OPENMP:
    {
      return "OpenMP";
    }

    case OPENCL:
    {
      return "OpenCL";
    }

    default:
    {
      Debug::getInstance ()->errorMessage ("Unknown backend selected");
    }
  }
}

namespace VariablePrefixes
{
  std::string const argsSizes = "argsSizes";
  std::string const dimensions = "dimensions";
  std::string const isFirstTime = "isFirstTime";
  std::string const planRet = "planRet";
  std::string const OP_DAT = "opDat";
  std::string const OP_MAP = "opMap";
  std::string const OP_ACCESS = "opAccess";
  std::string const OP_INDIRECTION = "opIndirection";
  std::string const cFortranPointer = "cFortranPointer";
}

namespace VariableSuffixes
{
  std::string const Size = "Size";
  std::string const Local = "Local";
  std::string const Global = "Global";
  std::string const Global = "Shared";
  std::string const Device = "Device";
  std::string const Host = "Host";
  std::string const Host = "Shared";
  std::string const Dimension = "Dimension";
}

std::string
VariableNames::getUserSubroutineName ()
{
  return "userSubroutine";
}

std::string
VariableNames::getOpSetName ()
{
  return "set";
}

std::string
VariableNames::getOpDatName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
VariableNames::getOpDatHostName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup) + VariableSuffixes::Host;
}

std::string
VariableNames::getOpDatLocalName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup) + VariableSuffixes::Local;
}

std::string
VariableNames::getOpDatGlobalName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup) + VariableSuffixes::Global;
}

std::string
VariableNames::getOpDatSharedName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup) + VariableSuffixes::Shared;
}

std::string
VariableNames::getOpDatSizeName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup) + VariableSuffixes::Size;
}

std::string
VariableNames::getOpDatDeviceName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup) + VariableSuffixes::Device;
}

std::string
VariableNames::getOpDatDimensionName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + VariableSuffixes::Dimension;
}

std::string
VariableNames::getOpIndirectionName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::OP_INDIRECTION + lexical_cast <string> (
      OP_DAT_ArgumentGroup);
}

std::string
VariableNames::getOpMapName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::OP_MAP
      + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
VariableNames::getOpAccessName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::OP_ACCESS + lexical_cast <string> (
      OP_DAT_ArgumentGroup);
}

std::string
VariableNames::getCToFortranVariableName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return VariablePrefixes::cFortranPointer + lexical_cast <string> (
      OP_DAT_ArgumentGroup);
}

std::string
VariableNames::getLocalToGlobalMappingName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectLoop::Fortran::VariablePrefixes::pindMaps + lexical_cast <
      string> (OP_DAT_ArgumentGroup);
}

std::string
VariableNames::getLocalToGlobalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectLoop::Fortran::VariablePrefixes::pindMaps + lexical_cast <
      string> (OP_DAT_ArgumentGroup) + VariableSuffixes::Size;
}

std::string
VariableNames::getGlobalToLocalMappingName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectLoop::Fortran::VariablePrefixes::pMaps
      + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
VariableNames::getGlobalToLocalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectLoop::Fortran::VariablePrefixes::pMaps
      + lexical_cast <string> (OP_DAT_ArgumentGroup) + VariableSuffixes::Size;
}

std::string
VariableNames::getNumberOfBytesVariableName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectLoop::Fortran::VariablePrefixes::numberOfBytes + lexical_cast <
      string> (OP_DAT_ArgumentGroup);
}

std::string
VariableNames::getRoundUpVariableName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectLoop::Fortran::VariablePrefixes::roundUp + lexical_cast <
      string> (OP_DAT_ArgumentGroup);
}

std::string
VariableNames::getIncrementAccessMapName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "argMap" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
VariableNames::getOpIndirectionSharedName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;
  
  return "ind_arg" + lexical_cast <string> (OP_DAT_ArgumentGroup) + "_s";
}

std::string
VariableNames::getIndirectionArgumentSizeName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "ind_arg" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
VariableNames::getDataSizesVariableDeclarationName (std::string const & suffix)
{
  return VariablePrefixes::argsSizes + "_" + suffix;
}

std::string
VariableNames::getDimensionsVariableDeclarationName (std::string const & suffix)
{
  return VariablePrefixes::dimensions + "_" + suffix;
}

std::string
VariableNames::getPlanReturnVariableDeclarationName (std::string const & suffix)
{
  return VariablePrefixes::planRet + "_" + suffix;
}

std::string
VariableNames::getFirstTimeExecutionVariableDeclarationName (
    std::string const & suffix)
{
  return VariablePrefixes::isFirstTime + "_" + suffix;
}

SgStatement *
SubroutineCalls::createCToFortranPointerCallStatement (
    SgScopeStatement * scope, SgExpression * parameter1,
    SgExpression * parameter2, SgExpression * parameter3)
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine ("c_f_pointer", scope);

  SgExprListExp * actualParameters = buildExprListExp (parameter1, parameter2,
      parameter3);

  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  return buildExprStatement (subroutineCall);
}
