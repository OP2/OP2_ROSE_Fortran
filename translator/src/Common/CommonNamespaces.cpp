#include <CommonNamespaces.h>
#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp>
#include <FortranTypesBuilder.h>
#include <rose.h>
#include <Exceptions.h>

namespace
{
  std::string const OpDatPrefix = "opDat";
  std::string const Size = "Size";
}

std::string
OP2::VariableNames::getUserSubroutineName ()
{
  return "userSubroutine";
}

std::string
OP2::VariableNames::getOpSetName ()
{
  return "set";
}

std::string
OP2::VariableNames::getOpDatName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
OP2::VariableNames::getOpDatHostName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Host";
}

std::string
OP2::VariableNames::getOpDatLocalName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Local";
}

std::string
OP2::VariableNames::getOpDatGlobalName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Global";
}

std::string
OP2::VariableNames::getOpDatSharedName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Shared";
}

std::string
OP2::VariableNames::getOpDatCardinalityName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "Cardinality";
}

std::string
OP2::VariableNames::getOpDatDeviceName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Device";
}

std::string
OP2::VariableNames::getOpDatDimensionName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "Dimension";
}

std::string
OP2::VariableNames::getOpIndirectionName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "opIndirection" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
OP2::VariableNames::getOpMapName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "opMap" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
OP2::VariableNames::getOpAccessName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "opAccess" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
OP2::VariableNames::getCToFortranVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "cFortranPointer" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
OP2::VariableNames::getLocalToGlobalMappingName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pindMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
OP2::VariableNames::getLocalToGlobalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pindMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size;
}

std::string
OP2::VariableNames::getGlobalToLocalMappingName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
OP2::VariableNames::getGlobalToLocalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size;
}

std::string
OP2::VariableNames::getNumberOfBytesVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "nBytes" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
OP2::VariableNames::getRoundUpVariableName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "roundUp" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
OP2::VariableNames::getIncrementAccessMapName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "argMap" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
OP2::VariableNames::getOpIndirectionSharedName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "ind_arg" + lexical_cast <string> (OP_DAT_ArgumentGroup) + "_s";
}

std::string
OP2::VariableNames::getIndirectionArgumentSizeName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "ind_arg" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
OP2::VariableNames::getPlanReturnVariableDeclarationName (
    std::string const & suffix)
{
  return "planRet_" + suffix;
}

std::string
OP2::VariableNames::getCUDASharedMemoryDeclarationName (SgType * type,
    unsigned int size)
{
  using boost::lexical_cast;
  using std::string;

  std::string const autoshared = "shared";

  switch (type->variantT ())
  {
    case V_SgTypeInt:
    {
      return autoshared + "Integer" + lexical_cast <string> (size);
    }
    case V_SgTypeFloat:
    {
      return autoshared + "Float" + lexical_cast <string> (size);
    }
    default:
    {
      throw Exceptions::CUDA::SharedVariableTypeException (
          "Unsupported type for autoshared variable: '" + type->class_name ());
    }
  }
}

std::string
OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (SgType * type,
    unsigned int size)
{
  using boost::lexical_cast;
  using std::string;

  std::string const autosharedOffset = "sharedOffset";

  switch (type->variantT ())
  {
    case V_SgTypeInt:
    {
      return autosharedOffset + "Integer" + lexical_cast <string> (size);
    }

    case V_SgTypeFloat:
    {
      return autosharedOffset + "Float" + lexical_cast <string> (size);
    }

    default:
    {
      throw Exceptions::CUDA::SharedVariableTypeException (
          "Unsupported type for autoshared variable: '" + type->class_name ());
    }
  }
}

std::string
OP2::VariableNames::getReductionArrayHostName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  string const prefix = "reductionArrayHost" + lexical_cast <string> (
      OP_DAT_ArgumentGroup);

  if (suffix.length () > 10)
  {
    boost::crc_32_type result;

    result.process_bytes (suffix.c_str (), suffix.length ());

    string const newSuffix = lexical_cast <string> (result.checksum ());

    return prefix + newSuffix;
  }

  return prefix + suffix;
}

std::string
OP2::VariableNames::getReductionArrayDeviceName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  string const prefix = "reductionArrayDevice" + lexical_cast <string> (
      OP_DAT_ArgumentGroup);

  if (suffix.length () > 10)
  {
    boost::crc_32_type result;

    result.process_bytes (suffix.c_str (), suffix.length ());

    string const newSuffix = lexical_cast <string> (result.checksum ());

    return prefix + newSuffix;
  }

  return prefix + suffix;
}

SgStatement *
SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
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
