


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


#include <CompilerGeneratedNames.h>
#include <boost/lexical_cast.hpp>
#include <boost/crc.hpp>
#include <FortranTypesBuilder.h>
#include "PlanFunctionNames.h"
#include <ParallelLoop.h>
#include <Exceptions.h>
#include <rose.h>

namespace
{
  std::string const OpDatPrefix = "opDat";
  std::string const Size = "Size";
}

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
ReductionVariableNames::getTemporaryReductionArrayName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "reductionTemporaryArray" + lexical_cast <string> (
      OP_DAT_ArgumentGroup);
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

std::string const
BooleanVariableNames::getFirstTimeExecutionVariableName (
    std::string const & suffix)
{
  return "firstTime_" + suffix;
}

std::string const
OP2VariableNames::getBlockSizeVariableName (std::string const & suffix)
{
  return "threadsPerBlockSize_" + suffix;
}

std::string const
OP2VariableNames::getPartitionSizeVariableName (std::string const & suffix)
{
  return "setPartitionSize_" + suffix;
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
OP2VariableNames::getOpSetCoreName ()
{
  return "opSetCore";
}

std::string const
OP2VariableNames::getOpSetCoreName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "opSet" + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Core";
}

std::string const
OP2VariableNames::getOpMatName (unsigned int n)
{
  using boost::lexical_cast;
  using std::string;

  return "opMat" + lexical_cast <string> (n);
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
OP2VariableNames::getOpDatCoreName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Core";
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
OP2VariableNames::getOpMapCoreName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "opMap" + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Core";
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
OP2VariableNames::getIndirectOpDatSizeName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "SharedIndirectionSize";
}

std::string const
OP2VariableNames::getIndirectOpDatMapName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "IndirectionMap";
}

std::string const
OP2VariableNames::getIndirectOpDatSharedMemoryName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return OpDatPrefix + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + "SharedIndirection";
}

std::string const
OP2VariableNames::getSharedMemoryDeclarationName (SgType * type,
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
OP2VariableNames::getSharedMemoryDeclarationName (std::string suffix)
{
  using std::string;

  std::string const prefix = "shared";

  return prefix + "_" + suffix;
}

std::string const
OP2VariableNames::getVolatileSharedMemoryDeclarationName (SgType * type,
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
OP2VariableNames::getSharedMemoryOffsetDeclarationName (SgType * type,
    unsigned int size)
{
  using boost::lexical_cast;
  using std::string;

  string const prefix = "sharedOffset";

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
OP2VariableNames::getSharedMemoryOffsetDeclarationName (std::string suffix)
{
  using std::string;

  string const prefix = "sharedOffset";

  return prefix + "_" + suffix;
}

std::string const
OP2VariableNames::getSharedMemoryPointerDeclarationName (SgType * type,
    unsigned int size)
{
  using boost::lexical_cast;
  using std::string;

  string const prefix = "sharedPointer";

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
OP2VariableNames::getSharedMemoryPointerDeclarationName (std::string suffix)
{
  using std::string;

  string const prefix = "sharedPointer";

  return prefix + "_" + suffix;
}

std::string const
OP2VariableNames::getPostfixNameAsConcatOfOpArgsNames (ParallelLoop * parallelLoop)
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::map;
  using std::string;
  using std::vector;
  using boost::lexical_cast;
  
  boost::crc_32_type result;
  
  string uniqueKernelName = "";
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    uniqueKernelName.append ("_");
    uniqueKernelName.append (parallelLoop->getOpDatVariableName (i));
  }
  
  result.process_bytes (uniqueKernelName.c_str (), uniqueKernelName.length ());
  
  string const postfixName = "_" + lexical_cast <string> (result.checksum ());
  
  return postfixName;
}
