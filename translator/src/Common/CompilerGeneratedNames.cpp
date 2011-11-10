#include <CompilerGeneratedNames.h>
#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp>
#include <FortranTypesBuilder.h>
#include <Exceptions.h>
#include <rose.h>

std::string const
ReductionVariableNames::getReductionArrayHostName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "reductionArrayHost" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
ReductionVariableNames::getReductionArrayDeviceName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "reductionArrayDevice" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
ReductionVariableNames::getReductionCardinalityName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "reductionCardinality" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
LoopVariableNames::getIterationCounterVariableName (unsigned int n)
{
  using boost::lexical_cast;
  using std::string;

  return "i" + lexical_cast <string> (n);
}

std::string const
LoopVariableNames::getUpperBoundVariableName (unsigned int n)
{
  using boost::lexical_cast;
  using std::string;

  return "n" + lexical_cast <string> (n);
}

namespace
{
  std::string const OpDatPrefix = "opDat";
  std::string const Size = "Size";
}

std::string const
OP2VariableNames::getUserSubroutineName ()
{
  return "userSubroutine";
}

std::string const
OP2VariableNames::getOpSetName ()
{
  return "set";
}

std::string const
OP2VariableNames::getOpDatName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2VariableNames::getOpDatHostName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Host";
}

std::string const
OP2VariableNames::getOpDatLocalName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Local";
}

std::string const
OP2VariableNames::getOpDatGlobalName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Global";
}

std::string const
OP2VariableNames::getOpDatCardinalityName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "Cardinality";
}

std::string const
OP2VariableNames::getOpDatDeviceName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Device";
}

std::string const
OP2VariableNames::getOpDatDimensionName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "Dimension";
}

std::string const
OP2VariableNames::getOpIndirectionName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "opIndirection" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2VariableNames::getOpMapName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "opMap" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2VariableNames::getOpAccessName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "opAccess" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2VariableNames::getCToFortranVariableName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "cFortranPointer" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2VariableNames::getLocalToGlobalMappingName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pindMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2VariableNames::getLocalToGlobalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pindMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size;
}

std::string const
OP2VariableNames::getGlobalToLocalMappingName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string const
OP2VariableNames::getGlobalToLocalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "pMaps" + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size;
}

std::string const
OP2VariableNames::getNumberOfBytesVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "nBytes";
}

std::string const
OP2VariableNames::getRoundUpVariableName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "RoundUp";
}

std::string const
OP2VariableNames::getIncrementAccessMapName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Map";
}

std::string const
OP2VariableNames::getIndirectionCUDASharedMemoryName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "SharedIndirection";
}

std::string const
OP2VariableNames::getIndirectionArgumentSizeName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "SharedIndirectionSize";
}

std::string const
OP2VariableNames::getIndirectionMapName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "IndirectionMap";
}

std::string const
OP2VariableNames::getPlanReturnVariableDeclarationName (
    std::string const & suffix)
{
  return "planRet_" + suffix;
}

std::string const
OP2VariableNames::getCUDASharedMemoryDeclarationName (SgType * type,
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
OP2VariableNames::getCUDAVolatileSharedMemoryDeclarationName (SgType * type,
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
OP2VariableNames::getCUDASharedMemoryOffsetDeclarationName (SgType * type,
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

