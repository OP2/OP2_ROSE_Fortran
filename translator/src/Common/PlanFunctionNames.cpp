#include "PlanFunctionNames.h"
#include <boost/lexical_cast.hpp>

namespace
{
  std::string const OpDatPrefix = "opDat";
  std::string const Size = "Size";
}

std::string const
PlanFunctionVariableNames::getActualPlanVariableName (
    std::string const & suffix)
{
  return actualPlan + suffix;
}

std::string const
PlanFunctionVariableNames::getPlanReturnVariableName (
    std::string const & suffix)
{
  return planRet + suffix;
}

std::string const
PlanFunctionVariableNames::getNumberOfThreadColoursPerBlockArrayName (
    std::string const & suffix)
{
  return nthrcol + suffix;
}

std::string const
PlanFunctionVariableNames::getThreadColourArrayName (std::string const & suffix)
{
  return thrcol + suffix;
}

std::string const
PlanFunctionVariableNames::getOffsetIntoBlockArrayName (
    std::string const & suffix)
{
  return offset + suffix;
}

std::string const
PlanFunctionVariableNames::getIndirectOpDatsLocalToGlobalMappingName (
    std::string const & suffix)
{
  return ind_maps + suffix;
}

std::string const
PlanFunctionVariableNames::getLocalToGlobalMappingName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  return ind_maps + lexical_cast <string> (OP_DAT_ArgumentGroup) + suffix;
}

std::string const
PlanFunctionVariableNames::getLocalToGlobalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  return ind_maps + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size
      + suffix;
}

std::string const
PlanFunctionVariableNames::getIndirectOpDatsOffsetArrayName (
    std::string const & suffix)
{
  return ind_offs + suffix;
}

std::string const
PlanFunctionVariableNames::getIndirectOpDatsNumberOfElementsArrayName (
    std::string const & suffix)
{
  return ind_sizes + suffix;
}

std::string const
PlanFunctionVariableNames::getOpDatsGlobalToLocalMappingName (
    std::string const & suffix)
{
  return maps + suffix;
}

std::string const
PlanFunctionVariableNames::getGlobalToLocalMappingName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  return maps + lexical_cast <string> (OP_DAT_ArgumentGroup) + suffix;
}

std::string const
PlanFunctionVariableNames::getGlobalToLocalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  return maps + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size + suffix;
}

std::string const
PlanFunctionVariableNames::getNumberOfSetElementsPerBlockArrayName (
    std::string const & suffix)
{
  return nelems + suffix;
}

std::string const
PlanFunctionVariableNames::getNumberOfBlockColoursName (
    std::string const & suffix)
{
  return ncolors + suffix;
}

std::string const
PlanFunctionVariableNames::getColourToNumberOfBlocksArrayName (
    std::string const & suffix)
{
  return ncolblk + suffix;
}

std::string const
PlanFunctionVariableNames::getColourToBlockArrayName (
    std::string const & suffix)
{
  return blkmap + suffix;
}

std::string const
PlanFunctionVariableNames::getIndirectionArgumentSizeName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "SharedIndirectionSize" + suffix;
}

std::string const
PlanFunctionVariableNames::getIndirectionMapName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "IndirectionMap" + suffix;
}
