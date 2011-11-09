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

SgFunctionCallExp *
OP2::CPPMacroSupport::createRoundUpCallStatement (SgScopeStatement * scope,
    SgExpression * parameterExpression)
{
  using namespace SageBuilder;
  using std::string;

  SgExprListExp * actualParameters = buildExprListExp (parameterExpression);

  string const functionName = "ROUND_UP";

  return buildFunctionCallExp (functionName, buildVoidType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OP2::CPPMacroSupport::createMaxCallStatement (SgScopeStatement * scope,
    SgExpression * parameterExpression1, SgExpression * parameterExpression2)
{
  using namespace SageBuilder;
  using std::string;

  SgExprListExp * actualParameters = buildExprListExp (parameterExpression1,
      parameterExpression2);

  string const functionName = "MAX";

  return buildFunctionCallExp (functionName, buildVoidType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OP2::CPPMacroSupport::createMinCallStatement (SgScopeStatement * scope,
    SgExpression * parameterExpression1, SgExpression * parameterExpression2)
{
  using namespace SageBuilder;
  using std::string;

  SgExprListExp * actualParameters = buildExprListExp (parameterExpression1,
      parameterExpression2);

  string const functionName = "MIN";

  return buildFunctionCallExp (functionName, buildVoidType (),
      actualParameters, scope);
}

std::string const
OP2::VariableNames::getUserSubroutineName ()
{
  return "userSubroutine";
}

std::string const
OP2::VariableNames::getOpSetName ()
{
  return "set";
}

std::string const
OP2::VariableNames::getOpDatName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2::VariableNames::getOpDatHostName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Host";
}

std::string const
OP2::VariableNames::getOpDatLocalName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Local";
}

std::string const
OP2::VariableNames::getOpDatGlobalName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Global";
}

std::string const
OP2::VariableNames::getOpDatSharedName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Shared";
}

std::string const
OP2::VariableNames::getOpDatCardinalityName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "Cardinality";
}

std::string const
OP2::VariableNames::getOpDatDeviceName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Device";
}

std::string const
OP2::VariableNames::getOpDatDimensionName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "Dimension";
}

std::string const
OP2::VariableNames::getOpIndirectionName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "opIndirection" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2::VariableNames::getOpMapName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "opMap" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2::VariableNames::getOpAccessName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "opAccess" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2::VariableNames::getCToFortranVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "cFortranPointer" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2::VariableNames::getLocalToGlobalMappingName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pindMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2::VariableNames::getLocalToGlobalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pindMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size;
}

std::string const
OP2::VariableNames::getGlobalToLocalMappingName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2::VariableNames::getGlobalToLocalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size;
}

std::string const
OP2::VariableNames::getNumberOfBytesVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "nBytes";
}

std::string const
OP2::VariableNames::getRoundUpVariableName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "RoundUp";
}

std::string const
OP2::VariableNames::getIncrementAccessMapName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Map";
}

std::string const
OP2::VariableNames::getIndirectionCUDASharedMemoryName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "SharedIndirection";
}

std::string const
OP2::VariableNames::getIndirectionArgumentSizeName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "SharedIndirectionSize";
}

std::string const
OP2::VariableNames::getIndirectionMapName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "IndirectionMap";
}

std::string const
OP2::VariableNames::getPlanReturnVariableDeclarationName (
    std::string const & suffix)
{
  return "planRet_" + suffix;
}

std::string const
OP2::VariableNames::getCUDASharedMemoryDeclarationName (SgType * type,
    unsigned int size)
{
  using boost::lexical_cast;
  using std::string;

  std::string const prefix = "shared";

  switch (type->variantT ())
  {
    case V_SgTypeInt:
    {
      return prefix + "Integer" + lexical_cast <string> (size);
    }
    case V_SgTypeFloat:
    {
      return prefix + "Float" + lexical_cast <string> (size);
    }
    case V_SgTypeDouble:
    {
      return prefix + "Double" + lexical_cast <string> (size);
    }
    default:
    {
      throw Exceptions::CUDA::SharedVariableTypeException (
          "Unsupported type for shared memory variable: '"
              + type->class_name ());
    }
  }
}

std::string const
OP2::VariableNames::getCUDAVolatileSharedMemoryDeclarationName (SgType * type,
    unsigned int size)
{
  using boost::lexical_cast;
  using std::string;

  std::string const prefix = "volatileShared";

  switch (type->variantT ())
  {
    case V_SgTypeInt:
    {
      return prefix + "Integer" + lexical_cast <string> (size);
    }
    case V_SgTypeFloat:
    {
      return prefix + "Float" + lexical_cast <string> (size);
    }
    case V_SgTypeDouble:
    {
      return prefix + "Double" + lexical_cast <string> (size);
    }
    default:
    {
      throw Exceptions::CUDA::SharedVariableTypeException (
          "Unsupported type for shared memory variable: '"
              + type->class_name ());
    }
  }
}

std::string const
OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (SgType * type,
    unsigned int size)
{
  using boost::lexical_cast;
  using std::string;

  std::string const prefix = "sharedOffset";

  switch (type->variantT ())
  {
    case V_SgTypeInt:
    {
      return prefix + "Integer" + lexical_cast <string> (size);
    }
    case V_SgTypeFloat:
    {
      return prefix + "Float" + lexical_cast <string> (size);
    }
    case V_SgTypeDouble:
    {
      return prefix + "Double" + lexical_cast <string> (size);
    }
    default:
    {
      throw Exceptions::CUDA::SharedVariableTypeException (
          "Unsupported type for shared memory variable: '"
              + type->class_name ());
    }
  }
}

std::string const
OP2::VariableNames::getReductionArrayHostName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "reductionArrayHost" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2::VariableNames::getReductionArrayDeviceName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "reductionArrayDevice" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2::VariableNames::getReductionCardinalityName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "reductionCardinality" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
CommonVariableNames::getIterationCounterVariableName (unsigned int n)
{
  using boost::lexical_cast;
  using std::string;

  return "i" + lexical_cast <string> (n);
}

std::string const
CommonVariableNames::getUpperBoundVariableName (unsigned int n)
{
  using boost::lexical_cast;
  using std::string;

  return "n" + lexical_cast <string> (n);
}
