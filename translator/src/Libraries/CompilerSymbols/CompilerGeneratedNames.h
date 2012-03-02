


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


#pragma once
#ifndef COMPILER_GENERATED_NAMES_H
#define COMPILER_GENERATED_NAMES_H

#include <string>

class SgType;
class SgStatement;
class SgExpression;
class SgScopeStatement;
class SgFunctionCallExp;
class ParallelLoop;

namespace ReductionVariableNames
{
  /*
   * ======================================================
   * Variable names used in reductions
   * ======================================================
   */

  std::string const reductionInput = "inputValue";
  std::string const reductionResult = "reductionResult";
  std::string const reductionOperation = "reductionOperation";
  std::string const reductionBytes = "reductionBytes";
  std::string const reductionSharedMemorySize = "reductionSharedMemorySize";

  std::string const maxBlocksPerGrid = "maxBlocksPerGrid";
  
  std::string const
  getReductionArrayHostName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getReductionArrayDeviceName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getReductionCardinalityName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getTemporaryReductionArrayName (unsigned int OP_DAT_ArgumentGroup);
}

namespace LoopVariableNames
{
  /*
   * ======================================================
   * Get a for loop counter variable name with this integer
   * suffix
   * ======================================================
   */

  std::string const
  getIterationCounterVariableName (unsigned int n);

  /*
   * ======================================================
   * Get an upper bound variable name with this integer
   * suffix
   * ======================================================
   */

  std::string const
  getUpperBoundVariableName (unsigned int n);
}

namespace BooleanVariableNames
{
  /*
   * ======================================================
   * Variable name to determine whether this is the first
   * time to execute a kernel
   * ======================================================
   */

  std::string const
  getFirstTimeExecutionVariableName (std::string const & suffix);
}

namespace OP2VariableNames
{
  std::string const localOffset = "localOffset";
  std::string const moduloResult = "moduloResult";
  std::string const nbytes = "nbytes";
  std::string const numberOfActiveThreads = "numberOfActiveThreads";
  std::string const numberOfActiveThreadsCeiling =
      "numberOfActiveThreadsCeiling";
  std::string const opDatCardinalities = "opDatCardinalities";
  std::string const opDatDimensions = "opDatDimensions";
  std::string const partitionSize = "partitionSize";
  std::string const setSize = "setSize";
  std::string const sharedMemoryOffset = "sharedMemoryOffset";
  std::string const sharedMemoryBytes = "sharedMemoryBytes";
  std::string const threadID = "threadID";
  std::string const threadItems = "threadItems";
  std::string const warpSize = "warpSize";
  std::string const warpMemorySize = "warpMemorySize";

  std::string const deviceString = "Device";
  
  std::string const calledTimes = "calledTimes";
  std::string const calledTimesString = "calledTimesString";
  std::string const returnDumpOpDat = "returnDumpOpDat";
  
  std::string const OP_PART_SIZE_1 = "OP_PART_SIZE_1";
  std::string const startTimeHost = "startTimeHost";
  std::string const endTimeHost = "endTimeHost";
  std::string const startTimeKernel = "startTimeKernel";
  std::string const endTimeKernel = "endTimeKernel";
  std::string const loopTimeHost = "loopTimeHost";
  std::string const loopTimeKernel = "loopTimeKernel";
  std::string const numberCalled = "numberCalled";
  std::string const istat = "istat";
  std::string const accumulatorHostTime = "accumulatorHostTime";
  std::string const accumulatorKernelTime = "accumulatorKernelTime";

  std::string const printProfilingInformation = "printProfInfo";


  std::string const
  getBlockSizeVariableName (std::string const & suffix);

  std::string const
  getPartitionSizeVariableName (std::string const & suffix);

  /*
   * ======================================================
   * Returns the name of the formal parameter with type
   * character array which models the name of the user
   * subroutine
   * ======================================================
   */
  std::string const
  getUserSubroutineName ();

  /*
   * ======================================================
   * Returns the name of the formal parameter with type
   * OP_SET
   * ======================================================
   */
  std::string const
  getOpSetName ();

  /*
   * ======================================================
   * Returns the name of the variable for the type OP_SET_CORE
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpSetCoreName ();

  /*
   * ======================================================
   * Returns the name of the variable for the type OP_SET_CORE
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpSetCoreName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Return the name of an OP_MAT variable in this argument 
   * group.
   * ======================================================
   */
  std::string const
  getOpMatName (unsigned int n);

  std::string const
  getOpMatEntryName (unsigned int n);

  /*
   * ======================================================
   * Returns the name of an OP_DAT variable in this OP_DAT
   * argument group
   * ======================================================
   */
  std::string const
  getOpDatName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of an OP_DAT variable in this OP_DAT
   * argument group on the host
   * ======================================================
   */
  std::string const
  getOpDatHostName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of a local OP_DAT variable in this
   * OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatLocalName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of a global OP_DAT variable in this
   * OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatGlobalName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the variable modelling the size of
   * an OP_DAT in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatCardinalityName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the OP_DAT device variable
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatDeviceName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the OP_DAT core variable
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatCoreName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the OP_DAT dimension variable
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpDatDimensionName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the indirection variable in this
   * OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpIndirectionName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the mapping variable in this OP_DAT
   * argument group
   * ======================================================
   */
  std::string const
  getOpMapName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the variable for the type OP_MAP_CORE
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getOpMapCoreName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the access variable in this OP_DAT
   * argument group
   * ======================================================
   */
  std::string const
  getOpAccessName (unsigned int OP_DAT_ArgumentGroup);

  /*
   * ======================================================
   * Returns the name of the C to Fortran variable
   * in this OP_DAT argument group
   * ======================================================
   */
  std::string const
  getCToFortranVariableName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getNumberOfBytesVariableName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getRoundUpVariableName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getIncrementAccessMapName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getIndirectOpDatSizeName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getIndirectOpDatMapName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getIndirectOpDatSharedMemoryName (unsigned int OP_DAT_ArgumentGroup);

  std::string const
  getSharedMemoryDeclarationName (SgType * type, unsigned int size);

  std::string const
  getSharedMemoryDeclarationName (std::string suffix);

  std::string const
  getVolatileSharedMemoryDeclarationName (SgType * type, unsigned int size);

  std::string const
  getSharedMemoryOffsetDeclarationName (SgType * type, unsigned int size);

  std::string const
  getSharedMemoryOffsetDeclarationName (std::string suffix);

  std::string const
  getSharedMemoryPointerDeclarationName (SgType * type, unsigned int size);

  std::string const
  getSharedMemoryPointerDeclarationName (std::string suffix);
  
  std::string const
  getPostfixNameAsConcatOfOpArgsNames (ParallelLoop * parallelLoop);
  
}

#endif
