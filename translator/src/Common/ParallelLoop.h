


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
#ifndef PARALLEL_LOOP_H
#define PARALLEL_LOOP_H

#include <string>
#include <map>
#include <vector>

class SgFunctionCallExp;
class SgType;
class Reduction;
class OpArgMatDefinition;
class OpIterationSpaceDefinition;

enum MAPPING_VALUE
{
  DIRECT, INDIRECT, GLOBAL
};

enum ACCESS_CODE_VALUE
{
  READ_ACCESS, WRITE_ACCESS, RW_ACCESS, INC_ACCESS, MAX_ACCESS, MIN_ACCESS
};

class ParallelLoop
{
  protected:

    /*
     * ======================================================
     * The function call expressions where calls to the
     * OP_PAR_LOOP occur. Note that there can be several
     * calls to the same kernel in different parts of the
     * code
     * ======================================================
     */
    std::vector <SgFunctionCallExp *> functionCallExpressions;

    /*
     * ======================================================
     * The name of the user subroutine kernel
     * ======================================================
     */
    std::string userSubroutineName;

    /*
     * ======================================================
     * The file names where the calls were detected
     * ======================================================
     */
    std::vector <std::string> fileNames;

    /*
     * ======================================================
     * The same OP_DAT variable can be passed as an argument
     * to OP_PAR_LOOP. This vector records the names of the
     * unique OP_DATs
     * ======================================================
     */
    std::vector <std::string> uniqueOpDats;

    /*
     * ======================================================
     * The base type of each OP_DAT. The map is indexed by
     * the argument group number
     * ======================================================
     */
    std::map <unsigned int, SgType *> OpDatTypes;

    /*
     * ======================================================
     * The dimension of each OP_DAT. The map is indexed by
     * the argument group number
     * ======================================================
     */
    std::map <unsigned int, unsigned int> OpDatDimensions;

    /*
     * ======================================================
     * The same OP_DAT variable can be passed as an argument
     * to OP_PAR_LOOP. This map records whether the OP_DAT argument
     * at this position (in the actual arguments) is a duplicate
     * of a previous OP_DAT argument
     * ======================================================
     */
    std::map <unsigned int, bool> OpDatDuplicates;
	
    /*
     * ======================================================
     * How is the data for the OP_DAT variables in this position
     * (in the actual arguments) accessed: through
     * a mapping or directly or as a global variable?
     * ======================================================
     */
    std::map <unsigned int, MAPPING_VALUE> OpDatMappingDescriptors;

    /*
     * ======================================================
     * How is the data for the OP_DAT variables in this position
     * (in the actual arguments) accessed: read, written, etc.
     * ======================================================
     */
    std::map <unsigned int, ACCESS_CODE_VALUE> OpDatAccessDescriptors;

    /*
     * ======================================================
     * What is OP_DAT variable name in this position
     * (in the actual arguments)?
     * ======================================================
     */
    std::map <unsigned int, std::string> OpDatVariableNames;

    /*
     * ======================================================
     * The reductions needed in this Parallel loop
     * ======================================================
     */
    std::vector <Reduction *> reductionsNeeded;

    /*
     * ======================================================
     * The number of OP_DAT argument groups
     * ======================================================
     */
    unsigned int numberOfOpDats;

    /*
     * Does this argument group actually describe an OP_ARG_MAT?
     */
    std::map <unsigned int, bool> actuallyOpMat;

    /*
     * Which op_arg_mat does this argument group correspond to?
     */
    std::map <unsigned int, unsigned int> opMatNumber;

    std::map <unsigned int, unsigned int> opDatNumber;
    /*
     * The actual op_arg_mat.
     */
    std::map <unsigned int, OpArgMatDefinition *> opMats;

    unsigned int numberOfOpMats;

    OpIterationSpaceDefinition * opItSpace;

  protected:

    ParallelLoop (SgFunctionCallExp * functionCallExpression);

  public:

    /*
     * ======================================================
     * Set the number of OP_DAT argument groups
     * ======================================================
     */
    void
    setNumberOfOpDatArgumentGroups (unsigned int numberOfOpDats);

    /*
     * ======================================================
     * How many OP_DAT argument groups are there?
     * ======================================================
     */
    unsigned int
    getNumberOfOpDatArgumentGroups ();

    void
    setNumberOfOpMatArgumentGroups (unsigned int n);

    unsigned int
    getNumberOfOpMatArgumentGroups ();

    unsigned int
    getNumberOfArgumentGroups ();
    /*
     * ======================================================
     * Checks the consistency of the OP_DAT arguments with
     * respect to their access values
     * ======================================================
     */
    void
    checkArguments ();

    /*
     * ======================================================
     * Is this parallel loop direct or indirect?
     * ======================================================
     */
    bool
    isDirectLoop ();

    void
    setOpDatType (unsigned int OP_DAT_ArgumentGroup, SgType * type);

    bool
    isArray (unsigned int OP_DAT_ArgumentGroup);

    bool
    isPointer (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * What is the base type of the (assumed) array type wrapped
     * by the OP_DAT declaration, in the particular OP_DAT
     * argument group?
     * ======================================================
     */
    SgType *
    getOpDatBaseType (unsigned int OP_DAT_ArgumentGroup);

    void
    setOpDatDimension (unsigned int OP_DAT_ArgumentGroup,
        unsigned int dimension);

    /*
     * ======================================================
     * What is the dimension of the OP_DAT variable in
     * this OP_DAT argument group?
     * ======================================================
     */
    unsigned int
    getOpDatDimension (unsigned int OP_DAT_ArgumentGroup);

    void
    setDuplicateOpDat (unsigned int OP_DAT_ArgumentGroup, bool value);

    /*
     * ======================================================
     * Is the OP_DAT variable in this OP_DAT argument group
     * the same as another OP_DAT in the actual arguments
     * passed to the OP_PAR_LOOP?
     * ======================================================
     */
    bool
    isDuplicateOpDat (unsigned int OP_DAT_ArgumentGroup);

    void
    setOpMapValue (unsigned int OP_DAT_ArgumentGroup, MAPPING_VALUE value);

    bool
    isIndirect (unsigned int OP_DAT_ArgumentGroup);

    bool
    isDirect (unsigned int OP_DAT_ArgumentGroup);

    bool
    isGlobal (unsigned int OP_DAT_ArgumentGroup);

    void
    setOpAccessValue (unsigned int OP_DAT_ArgumentGroup,
        ACCESS_CODE_VALUE value);

    bool
    isRead (unsigned int OP_DAT_ArgumentGroup);

    bool
    isWritten (unsigned int OP_DAT_ArgumentGroup);

    bool
    isReadAndWritten (unsigned int OP_DAT_ArgumentGroup);

    bool
    isIncremented (unsigned int OP_DAT_ArgumentGroup);

    bool
    isMaximised (unsigned int OP_DAT_ArgumentGroup);

    bool
    isMinimised (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * How many OP_DATs are indirectly accessed?
     * ======================================================
     */
    unsigned int
    getNumberOfIndirectOpDats ();

    /*
     * ======================================================
     * How many OP_DATs are indirectly accessed and are
     * unique in the actual arguments passed to the OP_PAR_LOOP?
     * ======================================================
     */
    unsigned int
    getNumberOfDistinctIndirectOpDats ();

    /*
     * ======================================================
     * Does this parallel loop have OP_DATs with increment
     * arguments?
     * ======================================================
     */
    bool
    hasIncrementedOpDats ();

    /*
     * ======================================================
     * Does the OP_DAT in this argument group require a reduction?
     * ======================================================
     */
    bool
    isReductionRequired (int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Does this parallel loop require a reduction?
     * ======================================================
     */
    bool
    isReductionRequired ();

    void
    setOpDatVariableName (unsigned int OP_DAT_ArgumentGroup,
        std::string const variableName);

    /*
     * ======================================================
     * What is the name of the OP_DAT variable in this OP_DAT
     * argument group
     * ======================================================
     */
    std::string
    getOpDatVariableName (unsigned int OP_DAT_ArgumentGroup);

    unsigned int
    getSizeOfOpDat (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * What is the maximum size of an individual element of an
     * OP_DAT in terms of bytes?
     * ======================================================
     */
    unsigned int
    getMaximumSizeOfOpDat ();

    bool
    isUniqueOpDat (std::string const & variableName);

    void
    setUniqueOpDat (std::string const & variableName);

    bool
    isOpMatArg (unsigned int n);

    void
    setIsOpMatArg (unsigned int n, bool isOpMat);

    void
    setOpMatArgNum (unsigned int n, unsigned int m);

    unsigned int
    getOpMatArgNum (unsigned int n);

    void
    setOpMatArg (unsigned int n, OpArgMatDefinition *);

    OpArgMatDefinition *
    getOpMatArg (unsigned int n);

    unsigned int
    getOpDatArgNum (unsigned int n);

    void
    setOpDatArgNum (unsigned int n, unsigned int m);

    /*
     * ======================================================
     * Does this OP_DAT argument group need its data to be
     * staged in from upper levels of the memory hierarchy
     * into lower levels?
     * ======================================================
     */
    bool
    isStageInNeeded (unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Adds an additional function call expression
     * ======================================================
     */
    void
    addFunctionCallExpression (SgFunctionCallExp * functionCallExpression);

    /*
     * ======================================================
     * Returns an iterator pointing to the first node in the
     * AST which models a call to this OP_PAR_LOOP
     * ======================================================
     */
    std::vector <SgFunctionCallExp *>::const_iterator
    getFirstFunctionCall ();

    /*
     * ======================================================
     * Returns an iterator pointing to the last node in the
     * AST which models a call to this OP_PAR_LOOP
     * ======================================================
     */
    std::vector <SgFunctionCallExp *>::const_iterator
    getLastFunctionCall ();

    /*
     * ======================================================
     * Adds an additional file name in which a call to this
     * OP_PAR_LOOP occurs
     * ======================================================
     */
    void
    addFileName (std::string fileName);

    /*
     * ======================================================
     * Returns an iterator pointing to the first file name
     * in which a call to this OP_PAR_LOOP is contained
     * ======================================================
     */
    std::vector <std::string>::const_iterator
    getFirstFileName ();

    /*
     * ======================================================
     * Returns an iterator pointing to the last file name
     * in which a call to this OP_PAR_LOOP is contained
     * ======================================================
     */
    std::vector <std::string>::const_iterator
    getLastFileName ();

    Reduction *
    getReductionTuple (unsigned int OP_DAT_ArgumentGroup);

    void
    getReductionsNeeded (std::vector <Reduction *> & reductions);

    std::string
    getUserSubroutineName ();

    /*
     * ======================================================
     * Get information describing the properties of this
     * OP_DAT argument group
     * ======================================================
     */
    std::string const
    getOpDatInformation (unsigned int OP_DAT_ArgumentGroup);

    OpIterationSpaceDefinition *
    getOpIterationSpace ();

    void
    setOpIterationSpace (OpIterationSpaceDefinition * itspace);
};

#endif
