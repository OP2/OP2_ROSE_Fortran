#include <boost/lexical_cast.hpp>
#include <CommonNamespaces.h>

namespace VariablePrefixes
{
  std::string const OP_DAT = "opDat";
  std::string const OP_MAP = "opMap";
  std::string const OP_ACCESS = "opAccess";
  std::string const OP_INDIRECTION = "opIndirection";
  std::string const CFortranPointer = "cFortranPointer";
}

namespace VariableSuffixes
{
  std::string const Size = "Size";
  std::string const Local = "Local";
  std::string const Global = "Global";
  std::string const Device = "Device";
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

  return VariablePrefixes::CFortranPointer + lexical_cast <string> (
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
      + lexical_cast <string> (OP_DAT_ArgumentGroup) + VariableSuffixes::Size;
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
