#include <ParallelLoop.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

ParallelLoop::ParallelLoop (SgFunctionCallExp * functionCallExpression) :
  functionCallExpression (functionCallExpression)
{
}

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

bool
ParallelLoop::isDirectLoop ()
{
  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (OpDatMappingDescriptors[i] == INDIRECT)
    {
      return false;
    }
  }
  return true;
}

unsigned int
ParallelLoop::getNumberOfDistinctIndirectOpDatArguments ()
{
  unsigned int count = 0;
  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (OpDatDuplicates[i] == false)
    {
      if (OpDatMappingDescriptors[i] == INDIRECT)
      {
        count++;
      }
    }
  }
  return count;
}

void
ParallelLoop::setOpDatType (unsigned int OP_DAT_ArgumentGroup, SgType * type)
{
  OpDatTypes[OP_DAT_ArgumentGroup] = type;
}

SgType *
ParallelLoop::getOpDatType (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatTypes[OP_DAT_ArgumentGroup];
}

SgType *
ParallelLoop::getOpDatBaseType (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  SgArrayType * isArrayType = isSgArrayType (OpDatTypes[OP_DAT_ArgumentGroup]);

  if (isArrayType == NULL)
  {
    Debug::getInstance ()->errorMessage ("OP_DAT argument '" + lexical_cast <
        string> (OP_DAT_ArgumentGroup)
        + "' is not array and therefore does not have a base type", __FILE__,
        __LINE__);
  }

  return isArrayType->get_base_type ();
}

void
ParallelLoop::setOpDatDimension (unsigned int OP_DAT_ArgumentGroup,
    unsigned int dimension)
{
  OpDatDimensions[OP_DAT_ArgumentGroup] = dimension;
}

unsigned int
ParallelLoop::getOpDatDimension (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatDimensions[OP_DAT_ArgumentGroup];
}

void
ParallelLoop::setDuplicateOpDat (unsigned int OP_DAT_ArgumentGroup, bool value)
{
  OpDatDuplicates[OP_DAT_ArgumentGroup] = value;
}

bool
ParallelLoop::isDuplicateOpDat (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatDuplicates[OP_DAT_ArgumentGroup];
}

void
ParallelLoop::setOpMapValue (unsigned int OP_DAT_ArgumentGroup,
    MAPPING_VALUE value)
{
  OpDatMappingDescriptors[OP_DAT_ArgumentGroup] = value;
}

MAPPING_VALUE
ParallelLoop::getOpMapValue (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatMappingDescriptors[OP_DAT_ArgumentGroup];
}

bool
ParallelLoop::isIndirect (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatMappingDescriptors[OP_DAT_ArgumentGroup] == INDIRECT;
}

bool
ParallelLoop::isDirect (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatMappingDescriptors[OP_DAT_ArgumentGroup] == DIRECT;
}

bool
ParallelLoop::isGlobal (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatMappingDescriptors[OP_DAT_ArgumentGroup] == GLOBAL;
}

bool
ParallelLoop::isGlobalScalar (unsigned int OP_DAT_ArgumentGroup)
{
  /*
   * ======================================================
   * Global scalar has the following properties:
   * 1) GLOBAL mapping descriptor
   * 2) Dimension of 1
   * 3) Base type NOT an array
   * ======================================================
   */

  return OpDatMappingDescriptors[OP_DAT_ArgumentGroup] == GLOBAL
      && OpDatDimensions[OP_DAT_ArgumentGroup] == 1 && isSgArrayType (
      OpDatTypes[OP_DAT_ArgumentGroup]) == NULL;
}

bool
ParallelLoop::isGlobalNonScalar (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatMappingDescriptors[OP_DAT_ArgumentGroup] == GLOBAL
      && OpDatDimensions[OP_DAT_ArgumentGroup] > 1;
}

void
ParallelLoop::setOpAccessValue (unsigned int OP_DAT_ArgumentGroup,
    ACCESS_CODE_VALUE value)
{
  OpDatAccessDescriptors[OP_DAT_ArgumentGroup] = value;
}

ACCESS_CODE_VALUE
ParallelLoop::getOpAccessValue (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatAccessDescriptors[OP_DAT_ArgumentGroup];
}

bool
ParallelLoop::isRead (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatAccessDescriptors[OP_DAT_ArgumentGroup] == READ_ACCESS;
}

bool
ParallelLoop::isWritten (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatAccessDescriptors[OP_DAT_ArgumentGroup] == WRITE_ACCESS;
}

bool
ParallelLoop::isReadAndWritten (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatAccessDescriptors[OP_DAT_ArgumentGroup] == RW_ACCESS;
}

bool
ParallelLoop::isIncremented (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatAccessDescriptors[OP_DAT_ArgumentGroup] == INC_ACCESS;
}

bool
ParallelLoop::isMaximised (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatAccessDescriptors[OP_DAT_ArgumentGroup] == MAX_ACCESS;
}

bool
ParallelLoop::isMinimised (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatAccessDescriptors[OP_DAT_ArgumentGroup] == MIN_ACCESS;
}

unsigned int
ParallelLoop::getNumberOfIndirectOpDats ()
{
  int count = 0;
  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (OpDatMappingDescriptors[i] == INDIRECT)
    {
      count++;
    }
  }
  return count;
}

unsigned int
ParallelLoop::getNumberOfDifferentIndirectOpDats ()
{
  int count = 0;
  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (OpDatMappingDescriptors[i] == INDIRECT && isDuplicateOpDat (i) == false)
    {
      count++;
    }
  }
  return count;
}

bool
ParallelLoop::isReductionRequired (int OP_DAT_ArgumentGroup)
{
  return OpDatMappingDescriptors[OP_DAT_ArgumentGroup] == GLOBAL
      && OpDatAccessDescriptors[OP_DAT_ArgumentGroup] != READ_ACCESS
      && isSgArrayType (OpDatTypes[OP_DAT_ArgumentGroup]) != NULL;
}

bool
ParallelLoop::isReductionRequired ()
{
  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (isReductionRequired (i))
    {
      return true;
    }
  }
  return false;
}

void
ParallelLoop::setOpDatVariableName (unsigned int OP_DAT_ArgumentGroup,
    std::string const variableName)
{
  OpDatVariableNames[OP_DAT_ArgumentGroup] = variableName;
}

std::string
ParallelLoop::getOpDatVariableName (unsigned int OP_DAT_ArgumentGroup)
{
  return OpDatVariableNames[OP_DAT_ArgumentGroup];
}

unsigned int
ParallelLoop::getSizeOfOpDat (unsigned int OP_DAT_ArgumentGroup)
{
  SgArrayType * arrayType = isSgArrayType (getOpDatType (OP_DAT_ArgumentGroup));

  SgType * baseType = arrayType->get_base_type ();

  SgIntVal * baseSize = isSgIntVal (baseType->get_type_kind ());

  return baseSize->get_value ();
}

unsigned int
ParallelLoop::getMaximumSizeOfOpDat ()
{
  unsigned int maximumSize = 0;

  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (isReductionRequired (i) == false && isGlobalNonScalar (i))
    {
      unsigned int sizeOfOpDat = getSizeOfOpDat (i);

      if (sizeOfOpDat > maximumSize)
      {
        maximumSize = sizeOfOpDat;
      }
    }
  }

  return maximumSize;
}

bool
ParallelLoop::isUniqueOpDat (std::string const & variableName)
{
  return find (uniqueOpDats.begin (), uniqueOpDats.end (), variableName)
      == uniqueOpDats.end ();
}

void
ParallelLoop::setUniqueOpDat (std::string const & variableName)
{
  uniqueOpDats.push_back (variableName);
}

SgExpressionPtrList &
ParallelLoop::getActualArguments ()
{
  return functionCallExpression->get_args ()->get_expressions ();
}

SgFunctionCallExp *
ParallelLoop::getFunctionCall ()
{
  return functionCallExpression;
}

std::string const &
ParallelLoop::getFileName () const
{
  return functionCallExpression->getFilenameString ();
}

Reduction *
ParallelLoop::getReductionTuple (unsigned int OP_DAT_ArgumentGroup)
{
  ROSE_ASSERT (isReductionRequired(OP_DAT_ArgumentGroup));

  SgArrayType * arrayType = isSgArrayType (getOpDatType (OP_DAT_ArgumentGroup));

  SgType * baseType = arrayType->get_base_type ();

  SgIntVal * baseSize = isSgIntVal (baseType->get_type_kind ());

  return new Reduction (arrayType, baseType, baseSize->get_value ());
}

void
ParallelLoop::getReductionsNeeded (std::vector <Reduction *> & reductions)
{
  using std::vector;

  for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (isReductionRequired (i))
    {
      Reduction * reduction = getReductionTuple (i);

      bool equivalentReductionFound = false;

      for (std::vector <Reduction *>::const_iterator it = reductions.begin (); it
          != reductions.end (); ++it)
      {
        if (reduction->isEquivalent (*it))
        {
          equivalentReductionFound = true;
          break;
        }
      }

      if (equivalentReductionFound == false)
      {
        reductions.push_back (reduction);
      }
    }
  }
}
