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
  if (suffix.length () == 0)
  {
    return actualPlan;
  }
  else
  {
    return actualPlan + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getPlanReturnVariableName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return planRet;
  }
  else
  {
    return planRet + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getNumberOfThreadColoursPerBlockArrayName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return nthrcol;
  }
  else
  {
    return nthrcol + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getNumberOfThreadColoursPerBlockSizeName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return nthrcol + Size;
  }
  else
  {
    return nthrcol + Size + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getThreadColourArrayName (std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return thrcol;
  }
  else
  {
    return thrcol + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getThreadColourSizeName (std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return thrcol + Size;
  }
  else
  {
    return thrcol + Size + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getOffsetIntoBlockArrayName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return offset;
  }
  else
  {
    return offset + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getOffsetIntoBlockSizeName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return offset + Size;
  }
  else
  {
    return offset + Size + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getIndirectOpDatsArrayName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return pnindirect;
  }
  else
  {
    return pnindirect + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getIndirectOpDatsLocalToGlobalMappingName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return ind_maps;
  }
  else
  {
    return ind_maps + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getLocalToGlobalMappingName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  if (suffix.length () == 0)
  {
    return ind_maps + lexical_cast <string> (OP_DAT_ArgumentGroup);
  }
  else
  {
    return ind_maps + lexical_cast <string> (OP_DAT_ArgumentGroup) + "_"
        + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getLocalToGlobalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  if (suffix.length () == 0)
  {
    return ind_maps + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size;
  }
  else
  {
    return ind_maps + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size + "_"
        + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getIndirectOpDatsOffsetArrayName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return ind_offs;
  }
  else
  {
    return ind_offs + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getIndirectOpDatsOffsetSizeName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return ind_offs + Size;
  }
  else
  {
    return ind_offs + Size + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getIndirectOpDatsNumberOfElementsArrayName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return ind_sizes;
  }
  else
  {
    return ind_sizes + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getIndirectOpDatsNumberOfElementsSizeName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return ind_sizes + Size;
  }
  else
  {
    return ind_sizes + Size + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getOpDatsGlobalToLocalMappingName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return mappingArray;
  }
  else
  {
    return mappingArray + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getGlobalToLocalMappingName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  if (suffix.length () == 0)
  {
    return mappingArray + lexical_cast <string> (OP_DAT_ArgumentGroup);
  }
  else
  {
    return mappingArray + lexical_cast <string> (OP_DAT_ArgumentGroup) + "_"
        + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getGlobalToLocalMappingSizeName (
    unsigned int OP_DAT_ArgumentGroup, std::string const & suffix)
{
  using boost::lexical_cast;
  using std::string;

  if (suffix.length () == 0)
  {
    return mappingArray + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size;
  }
  else
  {
    return mappingArray + lexical_cast <string> (OP_DAT_ArgumentGroup) + Size
        + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getNumberOfSetElementsPerBlockArrayName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return nelems;
  }
  else
  {
    return nelems + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getNumberOfSetElementsPerBlockSizeName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return nelems + Size;
  }
  else
  {
    return nelems + Size + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getNumberOfBlockColoursName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return ncolors;
  }
  else
  {
    return ncolors + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getColourToNumberOfBlocksArrayName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return ncolblk;
  }
  else
  {
    return ncolblk + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getColourToBlockArrayName (
    std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return blkmap;
  }
  else
  {
    return blkmap + "_" + suffix;
  }
}

std::string const
PlanFunctionVariableNames::getColourToBlockSizeName (std::string const & suffix)
{
  if (suffix.length () == 0)
  {
    return blkmap + Size;
  }
  else
  {
    return blkmap + Size + "_" + suffix;
  }
}
