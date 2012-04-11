


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <ParallelLoop.h>
#include <OP2Definitions.h>
#include <Reduction.h>
#include <Debug.h>
#include <Exceptions.h>
#include <rose.h>
#include <boost/lexical_cast.hpp>

ParallelLoop::ParallelLoop (SgFunctionCallExp * functionCallExpression)
{
  Debug::getInstance ()->debugMessage ("Parallel loop created",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  functionCallExpressions.push_back (functionCallExpression);

  SgExpressionPtrList & actualArguments =
      functionCallExpression->get_args ()->get_expressions ();

  SgFunctionRefExp * functionRefExpression = isSgFunctionRefExp (
      actualArguments.front ());

  userSubroutineName
      = functionRefExpression->getAssociatedFunctionDeclaration ()->get_name ().getString ();
}

void
ParallelLoop::setNumberOfOpDatArgumentGroups (unsigned int numberOfOpDats)
{
  this->numberOfOpDats = numberOfOpDats;
}

unsigned int
ParallelLoop::getNumberOfOpDatArgumentGroups ()
{
  return numberOfOpDats;
}

void
ParallelLoop::setNumberOfOpMatArgumentGroups (unsigned int n)
{
  this->numberOfOpMats = n;
}

unsigned int
ParallelLoop::getNumberOfOpMatArgumentGroups ()
{
  return numberOfOpMats;
}

unsigned int
ParallelLoop::getNumberOfArgumentGroups ()
{
  return numberOfOpDats + numberOfOpMats;
}

void
ParallelLoop::checkArguments ()
{
  using boost::lexical_cast;
  using std::string;

  for (unsigned int i = 1; i <= getNumberOfArgumentGroups (); ++i)
  {
    if (isOpMatArg (i)) continue;
    if (OpDatMappingDescriptors[i] == GLOBAL)
    {
      if (OpDatAccessDescriptors[i] == RW_ACCESS)
      {
        throw Exceptions::ParallelLoop::OpGblReadWriteException (
            "The READ/WRITE access descriptor of OP_GBL (in argument group "
                + lexical_cast <string> (i)
                + ") is not supported as its semantics are undefined");
      }
      else if (OpDatAccessDescriptors[i] == WRITE_ACCESS)
      {
        throw Exceptions::ParallelLoop::OpGblWriteException (
            "The WRITE access descriptor of OP_GBL (in argument group "
                + lexical_cast <string> (i)
                + ") is not supported as its semantics are undefined");
      }
    }
  }
}

bool
ParallelLoop::isDirectLoop ()
{
  for (unsigned int i = 1; i <= getNumberOfArgumentGroups (); ++i)
  {
    if (isOpMatArg (i) || OpDatMappingDescriptors[i] == INDIRECT)
    {
      return false;
    }
  }
  return true;
}

void
ParallelLoop::setOpDatType (unsigned int OP_DAT_ArgumentGroup, SgType * type)
{
  OpDatTypes[OP_DAT_ArgumentGroup] = type;
}

bool
ParallelLoop::isArray (unsigned int OP_DAT_ArgumentGroup)
{
  return isSgArrayType (OpDatTypes[OP_DAT_ArgumentGroup]);
}

bool
ParallelLoop::isPointer (unsigned int OP_DAT_ArgumentGroup)
{
  return isSgPointerType (OpDatTypes[OP_DAT_ArgumentGroup]);
}

SgType *
ParallelLoop::getOpDatBaseType (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  SgArrayType * isArrayType = isSgArrayType (OpDatTypes[OP_DAT_ArgumentGroup]);

  SgPointerType * isPointerType = isSgPointerType (
      OpDatTypes[OP_DAT_ArgumentGroup]);

  if (isArrayType != NULL)
  {
    return isArrayType->get_base_type ();
  }
  else if (isPointerType != NULL)
  {
    return isPointerType->get_base_type ();
  }
  else
  {
    return OpDatTypes[OP_DAT_ArgumentGroup];
  }
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

void
ParallelLoop::setOpAccessValue (unsigned int OP_DAT_ArgumentGroup,
    ACCESS_CODE_VALUE value)
{
  OpDatAccessDescriptors[OP_DAT_ArgumentGroup] = value;
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
  for (unsigned int i = 1; i <= getNumberOfArgumentGroups (); ++i)
  {
    if (isOpMatArg (i)) continue;
    if (OpDatMappingDescriptors[i] == INDIRECT)
    {
      count++;
    }
  }
  return count;
}

unsigned int
ParallelLoop::getNumberOfDistinctIndirectOpDats ()
{
  int count = 0;
  for (unsigned int i = 1; i <= getNumberOfArgumentGroups (); ++i)
  {
    if (isOpMatArg (i)) continue;
    if (isIndirect (i) && isDuplicateOpDat (i) == false)
    {
      count++;
    }
  }
  return count;
}

bool
ParallelLoop::hasIncrementedOpDats ()
{
  for (unsigned int i = 1; i <= getNumberOfArgumentGroups (); ++i)
  {
    if (isOpMatArg (i)) continue;
    if (isReductionRequired (i) == false && isIncremented (i))
    {
      return true;
    }
  }
  return false;
}

bool
ParallelLoop::isReductionRequired (int OP_DAT_ArgumentGroup)
{
  using std::string;
  using boost::lexical_cast;

  if ( isIncremented (OP_DAT_ArgumentGroup) ) 
    Debug::getInstance ()->debugMessage ("This argument is incremented", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  
  return OpDatMappingDescriptors[OP_DAT_ArgumentGroup] == GLOBAL
      && (isMaximised (OP_DAT_ArgumentGroup) || isMinimised (
          OP_DAT_ArgumentGroup) || isIncremented (OP_DAT_ArgumentGroup));
}

bool
ParallelLoop::isReductionRequired ()
{  
  for (unsigned int i = 1; i <= getNumberOfArgumentGroups (); ++i)
  {
    if (isOpMatArg (i)) continue;
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
  SgType * baseType = getOpDatBaseType (OP_DAT_ArgumentGroup);

  if (baseType->get_type_kind () == NULL)
  {
    if (isSgTypeShort (baseType))
    {
      return 2;
    }
    else if (isSgTypeInt (baseType))
    {
      return 4;
    }
    else if (isSgTypeLong (baseType))
    {
      return 8;
    }
    else if (isSgTypeFloat (baseType))
    {
      return 4;
    }
    else if (isSgTypeDouble (baseType))
    {
      return 8;
    }
    else
    {
      throw Exceptions::ParallelLoop::UnsupportedBaseTypeException ("");
    }
  }
  else
  {
    SgIntVal * baseSize = isSgIntVal (baseType->get_type_kind ());

    return baseSize->get_value ();
  }
}

unsigned int
ParallelLoop::getMaximumSizeOfOpDat ()
{
  using std::max;

  unsigned int maximumSize = 0;

  for (unsigned int i = 1; i <= getNumberOfArgumentGroups (); ++i)
  {
    if (isOpMatArg(i)) continue;
    maximumSize = max (maximumSize, getSizeOfOpDat (i));
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

bool
ParallelLoop::isOpMatArg (unsigned int n)
{
  return actuallyOpMat[n];
}

void
ParallelLoop::setIsOpMatArg (unsigned int n, bool isOpMat)
{
  actuallyOpMat[n] = isOpMat;
}

void
ParallelLoop::setOpMatArgNum (unsigned int n, unsigned int m)
{
  opMatNumber[n] = m;
}

unsigned int
ParallelLoop::getOpMatArgNum (unsigned int n)
{
  return opMatNumber[n];
}

void
ParallelLoop::setOpDatArgNum (unsigned int n, unsigned int m)
{
  opDatNumber[n] = m;
}

unsigned int
ParallelLoop::getOpDatArgNum (unsigned int n)
{
  return opDatNumber[n];
}

void
ParallelLoop::setOpMatArg (unsigned int n, OpArgMatDefinition *arg)
{
  opMats[n] = arg;
}

OpArgMatDefinition *
ParallelLoop::getOpMatArg (unsigned int n)
{
  return opMats[n];
}

bool
ParallelLoop::isStageInNeeded (unsigned int OP_DAT_ArgumentGroup)
{
  /*
   * ======================================================
   * We need a scratchpad variable when:
   * a) The data is to be reduced
   * b) The data is indirect AND incremented
   * c) The data is direct AND its dimension exceeds one
   * ======================================================
   */

  return isReductionRequired (OP_DAT_ArgumentGroup) || (isIndirect (
      OP_DAT_ArgumentGroup) && isIncremented (OP_DAT_ArgumentGroup))
      || (isDirect (OP_DAT_ArgumentGroup) && getOpDatDimension (
          OP_DAT_ArgumentGroup) > 1);
}

void
ParallelLoop::addFunctionCallExpression (
    SgFunctionCallExp * functionCallExpression)
{
  functionCallExpressions.push_back (functionCallExpression);
}

std::vector <SgFunctionCallExp *>::const_iterator
ParallelLoop::getFirstFunctionCall ()
{
  return functionCallExpressions.begin ();
}

std::vector <SgFunctionCallExp *>::const_iterator
ParallelLoop::getLastFunctionCall ()
{
  return functionCallExpressions.end ();
}

void
ParallelLoop::addFileName (std::string fileName)
{
  using std::find;

  if (find (fileNames.begin (), fileNames.end (), fileName) == fileNames.end ())
  {
    fileNames.push_back (fileName);
  }
}

std::vector <std::string>::const_iterator
ParallelLoop::getFirstFileName ()
{
  return fileNames.begin ();
}

std::vector <std::string>::const_iterator
ParallelLoop::getLastFileName ()
{
  return fileNames.end ();
}

Reduction *
ParallelLoop::getReductionTuple (unsigned int OP_DAT_ArgumentGroup)
{
  ROSE_ASSERT (isReductionRequired (OP_DAT_ArgumentGroup));

  SgType * baseType = getOpDatBaseType (OP_DAT_ArgumentGroup);

  unsigned int baseSize = getSizeOfOpDat (OP_DAT_ArgumentGroup);

  return new Reduction (baseType, baseSize);
}

void
ParallelLoop::getReductionsNeeded (std::vector <Reduction *> & reductions)
{
  using std::vector;

  for (unsigned int i = 1; i <= getNumberOfArgumentGroups (); ++i)
  {
    if (isOpMatArg(i)) continue;
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

std::string
ParallelLoop::getUserSubroutineName ()
{
  return userSubroutineName;
}

std::string const
ParallelLoop::getOpDatInformation (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  string data = "OP_DAT " + lexical_cast <string> (OP_DAT_ArgumentGroup) + " ";

  if (isRead (OP_DAT_ArgumentGroup))
  {
    data += "is read";
  }
  else if (isWritten (OP_DAT_ArgumentGroup))
  {
    data += "is written";
  }
  else if (isReadAndWritten (OP_DAT_ArgumentGroup))
  {
    data += "is read/written";
  }
  else if (isMinimised (OP_DAT_ArgumentGroup))
  {
    data += "is minimised";
  }
  else if (isMaximised (OP_DAT_ArgumentGroup))
  {
    data += "is maximised";
  }

  if (isGlobal (OP_DAT_ArgumentGroup))
  {
    data += ", OP_GBL ";
  }
  else
  {
    data += ", OP_DAT";
  }

  if (isArray (OP_DAT_ArgumentGroup))
  {
    data += ", ARRAY";
  }
  else if (isPointer (OP_DAT_ArgumentGroup))
  {
    data += ", POINTER";
  }
  else
  {
    data += ", SCALAR";
  }

  return data;
}

OpIterationSpaceDefinition *
ParallelLoop::getOpIterationSpace ()
{
  return opItSpace;
}

void
ParallelLoop::setOpIterationSpace (OpIterationSpaceDefinition * itspace)
{
  opItSpace = itspace;
}
