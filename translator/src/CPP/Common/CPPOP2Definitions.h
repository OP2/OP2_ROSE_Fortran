


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
#ifndef CPP_OP2_DEFINITIONS_H
#define CPP_OP2_DEFINITIONS_H

#include <OP2Definitions.h>
#include <rose.h>

class SgExprListExp;
class CPPProgramDeclarationsAndDefinitions;

class CPPImperialOpDatDefinition: public OpDatDefinition
{
    /*
     * ======================================================
     * Models an OP_DAT definition in C++ (Imperial API).
     *
     * The following function prototype is assumed:
     * OP_DECL_DAT (op_set, int, template<T> *)
     * ======================================================
     */

  public:

    static int const indexOpSet = 0;

    static int const indexDimension = 1;

    static int const indexDataArray = 2;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 3;
    }

    CPPImperialOpDatDefinition (SgExprListExp * parameters,
        std::string const & variableName);
};

class CPPImperialOpSetDefinition: public OpSetDefinition
{
    /*
     * ======================================================
     * Models an OP_SET definition in C++ (Imperial API).
     *
     * The following function prototype is assumed:
     * OP_DECL_SET (int)
     * ======================================================
     */

  public:

    static int const indexDimension = 0;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 1;
    }

    CPPImperialOpSetDefinition (SgExprListExp * parameters,
        std::string const & variableName);
};

class CPPOxfordOpSubSetDefinition: public OpSubSetDefinition
{
    /*
     * ======================================================
     * Models an OP_SUBSET definition in C++ (Oxford API).
     *
     * The following function prototype is assumed:
     * OP_DECL_SUBSET (op_set, givenname, kernel, op_arg...)
     * ======================================================
     */
    
private:
    SgExprListExp* parameters;
	SgFunctionDefinition* filterFunction;
	SgFunctionDefinition* wrapperFunction;
	SgVariableDeclaration * subsetDeclaration;
    
public:
    static int const indexOriginSet = 0;
	static int const indexGivenName = 1;
    static int const indexFilterKernel = 2;
    
    SgNode* getOpArgDat(int i);
	
	SgFunctionDefinition* getFilterFunction ();
	SgFunctionDefinition* getFilterWrapperFunction ();
	void setFilterWrapperFunction (SgFunctionDefinition* wrapper);
	SgVariableDeclaration * getSubsetDeclaration ();
    
    CPPOxfordOpSubSetDefinition (SgExprListExp * parameters,
                                std::string const & variableName,
								SgVariableDeclaration * subsetDeclaration);
};

class CPPImperialOpMapDefinition: public OpMapDefinition
{
    /*
     * ======================================================
     * Models an OP_MAP definition in C++ (Imperial API).
     *
     * The following function prototype is assumed:
     * OP_DECL_MAP (op_set, op_set, int, int *)
     * ======================================================
     */

  public:

    static int const indexOpSetSource = 0;

    static int const indexOpSetDestination = 1;

    static int const indexDimension = 2;

    static int const indexMappingArray = 3;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 4;
    }

    CPPImperialOpMapDefinition (SgExprListExp * parameters,
        std::string const & variableName);
};

class CPPImperialOpConstDefinition: public OpConstDefinition
{
    /*
     * ======================================================
     * Models a constant definition in C++ (Imperial API).
     *
     * The following function prototype is assumed:
     * OP_DECL_CONST (int, template<T> *)
     * ======================================================
     */

  public:

    static int const indexDimenson = 0;

    static int const indexData = 1;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 2;
    }

    CPPImperialOpConstDefinition (SgExprListExp * parameters);
};

class CPPImperialOpArgDatCall
{
    /*
     * ======================================================
     * Models a call to OP_ARG_DAT in C++ (Imperial API).
     *
     * The following function prototype is assumed:
     * OP_ARG_DAT (op_dat, int, op_map, op_access)
     * ======================================================
     */

  public:

    static int const indexOpDat = 0;

    static int const indexOpIndex = 1;

    static int const indexOpMap = 2;

    static int const indexAccessDescriptor = 3;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 4;
    }
};

class CPPImperialOpArgGblCall
{
    /*
     * ======================================================
     * Models a call to OP_ARG_GBL in C++ (Imperial API).
     *
     * The following function prototype is assumed:
     * OP_ARG_GBL (op_dat, op_access)
     * ======================================================
     */

  public:

    static int const indexOpDat = 0;

    static int const indexAccessDescriptor = 1;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 2;
    }
};

class CPPOxfordOpDatDefinition: public OpDatDefinition
{
    /*
     * ======================================================
     * Models an OP_DAT definition in C++ (Oxford API).
     *
     * The following function prototype is assumed:
     * OP_DECL_DAT (op_set, int, char *, template<T> *, char *)
     * ======================================================
     */

  public:

    static int const indexOpSet = 0;

    static int const indexDimension = 1;

    static int const indexTypeName = 2;

    static int const indexDataArray = 3;

    static int const indexOpDatName = 4;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 5;
    }

    CPPOxfordOpDatDefinition (SgExprListExp * parameters,
        std::string const & variableName);
};

class CPPOxfordOpSetDefinition: public OpSetDefinition
{
    /*
     * ======================================================
     * Models an OP_SET definition in C++ (Oxford API).
     *
     * The following function prototype is assumed:
     * OP_DECL_SET (int, char *)
     * ======================================================
     */

  public:

    static int const indexDimension = 0;

    static int const indexOpSetName = 1;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 2;
    }

    CPPOxfordOpSetDefinition (SgExprListExp * parameters,
        std::string const & variableName);
};

class CPPOxfordOpMapDefinition: public OpMapDefinition
{
    /*
     * ======================================================
     * Models an OP_MAP definition in C++ (Oxford API).
     *
     * The following function prototype is assumed:
     * OP_DECL_MAP (op_set, op_set, int, int *, char *)
     * ======================================================
     */

  public:

    static int const indexOpSetSource = 0;

    static int const indexOpSetDestination = 1;

    static int const indexDimension = 2;

    static int const indexMappingArray = 3;

    static int const indexOpMapName = 4;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 5;
    }

    CPPOxfordOpMapDefinition (SgExprListExp * parameters,
        std::string const & variableName);
};

class CPPOxfordOpConstDefinition: public OpConstDefinition
{
    /*
     * ======================================================
     * Models a constant definition in C++ (Oxford API).
     *
     * The following function prototype is assumed:
     * OP_DECL_CONST (int, char *, template<T> *)
     * ======================================================
     */

  public:

    static int const indexDimension = 0;

    static int const indexTypeName = 1;

    static int const indexData = 2;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 3;
    }

    CPPOxfordOpConstDefinition (SgExprListExp * parameters);
};

class CPPOxfordOpArgDatCall
{
    /*
     * ======================================================
     * Models a call to OP_ARG_DAT in C++ (Oxford API).
     *
     * The following function prototype is assumed:
     * OP_ARG_DAT (op_dat, int, op_map, int, char *, op_access)
     * ======================================================
     */

  public:

    static int const indexOpDat = 0;

    static int const indexOpIndex = 1;

    static int const indexOpMap = 2;

    static int const indexDimension = 3;

    static int const indexTypeName = 4;

    static int const indexAccessDescriptor = 5;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 6;
    }
};

class CPPOxfordOpArgGblCall
{
    /*
     * ======================================================
     * Models a call to OP_ARG_GBL in C++ (Oxford API).
     *
     * The following function prototype is assumed:
     * OP_ARG_GBL (op_dat, int, char *, op_access)
     * ======================================================
     */

  public:

    static int const indexOpDat = 0;

    static int const indexDimension = 1;

    static int const indexTypeName = 2;

    static int const indexAccessDescriptor = 3;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 4;
    }
};

class CPPImperialOpArgMatDefinition: public OpArgMatDefinition
{
    /*
     * op_arg_mat(op_mat, idx1, map1, idx2, map2, access)
     */

  public:

    static int const indexOpMat = 0;

    static int const indexOpIndex1 = 1;

    static int const indexOpMap1 = 2;

    static int const indexOpIndex2 = 3;

    static int const indexOpMap2 = 4;

    static int const indexAccessDescriptor = 5;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 6;
    }

    CPPImperialOpArgMatDefinition (SgExprListExp * parameters,
        CPPProgramDeclarationsAndDefinitions * declarations,
        OpIterationSpaceDefinition * itspace);
};

class CPPOxfordOpArgMatDefinition: public OpArgMatDefinition
{
    /*
     * op_arg_mat(op_mat, idx1, map1, idx2, map2, dim, "datatype", access)
     */

  public:

    static int const indexOpMat = 0;

    static int const indexOpIndex1 = 1;

    static int const indexOpMap1 = 2;

    static int const indexOpIndex2 = 3;

    static int const indexOpMap2 = 4;

    static int const indexDimension = 5;

    static int const indexTypeName = 6;

    static int const indexAccessDescriptor = 7;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 8;
    }

    CPPOxfordOpArgMatDefinition (SgExprListExp * parameters,
        CPPProgramDeclarationsAndDefinitions * declarations,
        OpIterationSpaceDefinition * itspace);
};

class CPPImperialOpSparsityDefinition: public OpSparsityDefinition
{
    /*
     * op_decl_sparsity(op_map, op_map);
     */
  public:

    static int const indexOpMap1 = 0;

    static int const indexOpMap2 = 1;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 2;
    }

    CPPImperialOpSparsityDefinition (
        SgExprListExp * parameters, std::string const & variableName);
};

class CPPImperialOpMatDefinition: public OpMatDefinition
{

    /*
     * op_decl_mat(sparsity, dimension, "datatype")
     */
  public:

    static int const indexOpSparsity = 0;

    static int const indexDimension = 1;

    static int const indexTypeName = 2;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 3;
    }

    CPPImperialOpMatDefinition (
        SgExprListExp * parameters, std::string const & variableName);
};

class CPPOxfordOpSparsityDefinition: public OpSparsityDefinition
{

    /*
     * op_decl_sparsity(map1, map2, "name")
     */

  public:

    static int const indexOpMap1 = 0;

    static int const indexOpMap2 = 1;

    static int const indexOpSparsityName = 2;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 3;
    }

    CPPOxfordOpSparsityDefinition (
        SgExprListExp * parameters, std::string const & variableName);
};

class CPPOxfordOpMatDefinition: public OpMatDefinition
{
    /*
     * op_decl_mat(sparsity, dimension, "typename", sizeof(typename), "name")
     */

  public:

    static int const indexOpSparsity = 0;

    static int const indexDimension = 1;

    static int const indexTypeName = 2;

    static int const indexTypeSize = 3;

    static int const indexOpMatName = 4;

  public:

    static unsigned int
    getNumberOfExpectedArguments ()
    {
      return 5;
    }

    CPPOxfordOpMatDefinition (
        SgExprListExp * parameters, std::string const & variableName);
};

#endif
