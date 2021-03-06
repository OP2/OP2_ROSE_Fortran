


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

#include <OP2.h>
#include <CPPOP2Definitions.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <rose.h>

CPPImperialOpDatDefinition::CPPImperialOpDatDefinition (
    SgExprListExp * parameters, std::string const & variableName)
{
  using boost::lexical_cast;
  using std::string;

  this->variableName = variableName;

  opSetName
      = isSgVarRefExp (parameters->get_expressions ()[indexOpSet])->get_symbol ()->get_name ().getString ();

  dimension
      = isSgIntVal (parameters->get_expressions ()[indexDimension])->get_value ();

  baseType
      = isSgVarRefExp (parameters->get_expressions ()[indexDataArray])->get_type ();

  ROSE_ASSERT (opSetName.empty () == false);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (baseType != NULL);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_DAT declaration: '"
      + variableName + "'. The data pertains to the set '" + opSetName
      + "'. Its actual type is " + baseType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

CPPImperialOpSetDefinition::CPPImperialOpSetDefinition (
    SgExprListExp * parameters, std::string const & variableName)
{
  this->variableName = variableName;

  if (isSgVarRefExp (parameters->get_expressions ()[indexDimension]))
  {
    dimensionName
      = isSgVarRefExp (parameters->get_expressions ()[indexDimension])->get_symbol ()->get_name ().getString ();
  }
  else
  {
    dimensionName = "__literalSetSize";
  }

  ROSE_ASSERT (dimensionName.empty () == false);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_SET declaration: '"
      + variableName + "'. Its dimension is contained in '" + dimensionName
      + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__ );
}

SgNode*
CPPOxfordOpSubSetDefinition::getOpArgDat(int i) {
    return this->parameters->get_expressions ()[3 + i];
}

SgFunctionDefinition* 
CPPOxfordOpSubSetDefinition::getFilterFunction ()
{
	return this->filterFunction;
}

SgFunctionDefinition* 
CPPOxfordOpSubSetDefinition::getFilterWrapperFunction ()
{
	return this->wrapperFunction;
}
void
CPPOxfordOpSubSetDefinition::setFilterWrapperFunction (SgFunctionDefinition* wrapper)
{
	this->wrapperFunction = wrapper;
}

SgVariableDeclaration * 
CPPOxfordOpSubSetDefinition::getSubsetDeclaration () {
	return this->subsetDeclaration;
}

CPPOxfordOpSubSetDefinition::CPPOxfordOpSubSetDefinition (
                                                        SgExprListExp * parameters, std::string const & variableName,
														  SgVariableDeclaration * subsetDeclaration)
{
    this->variableName = variableName;
    
    this->originSetName = isSgVarRefExp (parameters->get_expressions ()[indexOriginSet])->get_symbol ()->get_name ().getString ();
	
	this->subsetDeclaration = subsetDeclaration;
    
    this->filterKernelName = isSgFunctionRefExp (parameters->get_expressions ()[indexFilterKernel])->get_symbol ()->get_name ().getString ();
	this->filterFunction = isSgFunctionRefExp (parameters->get_expressions ()[indexFilterKernel])->getAssociatedFunctionDeclaration ()->get_definition ();
    
    this->nbFilterArg = parameters->get_expressions ().size () - 3;
    this->parameters = parameters;
    
    Debug::getInstance ()->debugMessage ("Found an OP_SUBSET declaration: '"
                                         + variableName + "', from '" + originSetName + "', filtered with '" + filterKernelName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__ );
}

CPPImperialOpMapDefinition::CPPImperialOpMapDefinition (
    SgExprListExp * parameters, std::string const & variableName)
{
  using boost::lexical_cast;
  using std::string;

  this->variableName = variableName;

  sourceOpSetName
      = isSgVarRefExp (parameters->get_expressions ()[indexOpSetSource])->get_symbol ()->get_name ().getString ();

  destinationOpSetName
      = isSgVarRefExp (parameters->get_expressions ()[indexOpSetDestination])->get_symbol ()->get_name ().getString ();

  dimension
      = isSgIntVal (parameters->get_expressions ()[indexDimension])->get_value ();

  mappingName
      = isSgVarRefExp (parameters->get_expressions ()[indexMappingArray])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (sourceOpSetName.empty () == false);
  ROSE_ASSERT (destinationOpSetName.empty () == false);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (mappingName.empty () == false);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_MAP declaration: '"
      + variableName + "'. Mapping from '" + sourceOpSetName + "' to '"
      + destinationOpSetName + "' through '" + mappingName + "' with "
      + lexical_cast <string> (dimension) + " mappings per element",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

CPPImperialOpConstDefinition::CPPImperialOpConstDefinition (
    SgExprListExp * parameters)
{
  using boost::lexical_cast;
  using std::string;

  dimension
      = isSgIntVal (parameters->get_expressions ()[indexDimenson])->get_value ();

  SgAddressOfOp * addressOfOperator = isSgAddressOfOp (
      parameters->get_expressions ()[indexData]);

  if (addressOfOperator != NULL)
  {
    SgVarRefExp * operandExpression = isSgVarRefExp (
        addressOfOperator->get_operand ());

    ROSE_ASSERT (operandExpression != NULL);

    variableName = operandExpression->get_symbol ()->get_name ().getString ();
  }
  else
  {
    variableName
        = isSgVarRefExp (parameters->get_expressions ()[indexData])->get_symbol ()->get_name ().getString ();
  }

  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_CONST declaration: '"
      + variableName + "' Its dimension is "
      + lexical_cast <string> (dimension), Debug::FUNCTION_LEVEL, __FILE__,
      __LINE__);
}

CPPOxfordOpDatDefinition::CPPOxfordOpDatDefinition (SgExprListExp * parameters,
    std::string const & variableName)
{
  using boost::lexical_cast;
  using std::string;

  this->variableName = variableName;

  opSetName
      = isSgVarRefExp (parameters->get_expressions ()[indexOpSet])->get_symbol ()->get_name ().getString ();

  dimension
      = isSgIntVal (parameters->get_expressions ()[indexDimension])->get_value ();

  baseType
      = isSgVarRefExp (parameters->get_expressions ()[indexDataArray])->get_type ();

  ROSE_ASSERT (opSetName.empty () == false);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (baseType != NULL);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_DAT declaration: '"
      + variableName + "'. The data pertains to the set '" + opSetName
      + "'. Its actual type is " + baseType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

CPPOxfordOpSetDefinition::CPPOxfordOpSetDefinition (SgExprListExp * parameters,
    std::string const & variableName)
{
  this->variableName = variableName;

  if (isSgVarRefExp (parameters->get_expressions ()[indexDimension]))
  {
    dimensionName
      = isSgVarRefExp (parameters->get_expressions ()[indexDimension])->get_symbol ()->get_name ().getString ();
  }
  else
  {
    dimensionName = "__literalSetSize";
  }
  ROSE_ASSERT (dimensionName.empty () == false);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_SET declaration: '"
      + variableName + "'. Its dimension is contained in '" + dimensionName
      + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__ );
}

CPPOxfordOpMapDefinition::CPPOxfordOpMapDefinition (SgExprListExp * parameters,
    std::string const & variableName)
{
  using boost::lexical_cast;
  using std::string;

  SgExpressionPtrList & parameterExpressions = parameters->get_expressions ();

  this->variableName = variableName;

  sourceOpSetName
      = isSgVarRefExp (parameterExpressions[indexOpSetSource])->get_symbol ()->get_name ().getString ();

  destinationOpSetName
      = isSgVarRefExp (parameterExpressions[indexOpSetDestination])->get_symbol ()->get_name ().getString ();

  dimension = isSgIntVal (parameterExpressions[indexDimension])->get_value ();

  mappingName
      = isSgVarRefExp (parameterExpressions[indexMappingArray])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (sourceOpSetName.empty () == false);
  ROSE_ASSERT (destinationOpSetName.empty () == false);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (mappingName.empty () == false);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_MAP declaration: '"
      + variableName + "'. Mapping from '" + sourceOpSetName + "' to '"
      + destinationOpSetName + "' through '" + mappingName + "' with "
      + lexical_cast <string> (dimension) + " mappings per element",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

CPPOxfordOpConstDefinition::CPPOxfordOpConstDefinition (
    SgExprListExp * parameters)
{
  using namespace SageBuilder;
  using boost::iequals;
  using boost::lexical_cast;
  using std::string;

  SgExpressionPtrList & parameterExpressions = parameters->get_expressions ();

  dimension = isSgIntVal (parameterExpressions[indexDimension])->get_value ();

  SgAddressOfOp * addressOfOperator = isSgAddressOfOp (
      parameterExpressions[indexData]);

  if (addressOfOperator != NULL)
  {
    SgVarRefExp * operandExpression = isSgVarRefExp (
        addressOfOperator->get_operand ());

    ROSE_ASSERT (operandExpression != NULL);

    variableName = operandExpression->get_symbol ()->get_name ().getString ();

    baseType = operandExpression->get_type ();
  }
  else
  {
    SgVarRefExp * varRefExpression = isSgVarRefExp (
        parameters->get_expressions ()[indexData]);

    ROSE_ASSERT (varRefExpression != NULL);

    variableName = varRefExpression->get_symbol ()->get_name ().getString ();

    baseType = varRefExpression->get_type ();
  }

  ROSE_ASSERT (baseType != NULL);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_CONST declaration: '"
      + variableName + "' Its dimension is "
      + lexical_cast <string> (dimension), Debug::FUNCTION_LEVEL, __FILE__,
      __LINE__);
}

CPPImperialOpSparsityDefinition::CPPImperialOpSparsityDefinition (
    SgExprListExp * parameters, std::string const & variableName)
{
  using std::string;
  this->variableName = variableName;

  map1Name = isSgVarRefExp (
      parameters->get_expressions ()[indexOpMap1])->get_symbol ()->
      get_name ().getString ();

  map2Name = isSgVarRefExp (
      parameters->get_expressions ()[indexOpMap2])->get_symbol ()->
      get_name ().getString ();

  ROSE_ASSERT (variableName.empty () == false);
  ROSE_ASSERT (map1Name.empty () == false);
  ROSE_ASSERT (map2Name.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_SPARSITY declaration: '"
      + variableName + "' using maps " + map1Name + " and " + map2Name,
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

CPPImperialOpMatDefinition::CPPImperialOpMatDefinition (
    SgExprListExp * parameters, std::string const & variableName)
{
  using std::string;
  using boost::iequals;
  using boost::lexical_cast;

  this->variableName = variableName;

  sparsityName = isSgVarRefExp (
      parameters->get_expressions ()[indexOpSparsity])->get_symbol ()->
      get_name ().getString ();

  dimension = isSgIntVal (
      parameters->get_expressions ()[indexDimension])->get_value ();

  string type = isSgStringVal (
      isSgCastExp (
          parameters->get_expressions ()[indexTypeName])->get_operand ())->get_value ();

  /*
   * FIXME: is there a better way to derive the base type?
   */
  if (iequals (type, "float"))
  {
    baseType = new SgTypeFloat ();
  }
  else if (iequals(type, "double"))
  {
    baseType = new SgTypeDouble ();
  }
  else
  {
    Debug::getInstance ()->debugMessage (
        "Unknown OP_MAT entry type '" + type + "', aborting",
        Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);
    ROSE_ASSERT (false);
  }

  Debug::getInstance ()->debugMessage (
      "Found an OP_MAT declaration: it has a sparsity pattern given by '" +
      sparsityName + "', dimension " + lexical_cast <string> (dimension) +
      " and stores entries of type " + baseType->class_name (),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

static void getArgMatExtent (SgExpression * idx,
    OpIterationSpaceDefinition * &itspace,
    int mapDim,
    int & mapidx,
    int idxval,
    bool createItspace)
{
  using boost::iequals;
  SgFunctionCallExp * f = isSgFunctionCallExp (idx);
  int extent[2];
  /* Reference to op_iteration_space argument */
  if (f != NULL)
  {
    ROSE_ASSERT (iequals (OP2::OP_I,
            f->getAssociatedFunctionSymbol ()->get_name ().getString ()));
    int which = isSgIntVal(f->get_args ()->get_expressions ()[0])->
        get_value ();

    /*
     * First argument to op_iteration_space is the set, which we don't
     * want, so shift user values by one.
     */
    mapidx = --which;
  }
  else if (isSgIntVal (idx) && isSgIntVal (idx)->get_value () < 0)
  {
    /* Negative, OP_ALL */
    mapidx = idxval;
    extent[0] = 0;
    extent[1] = mapDim;
  }
  else
  {
    Debug::getInstance ()->debugMessage (
        "Unable to parse index in OP_ARG_MAT (not OP_ALL or op_i)",
        Debug::LOWEST_DEBUG_LEVEL, __FILE__, __LINE__);
    ROSE_ASSERT (false);
  }

  if (createItspace)
  {
    if (itspace == NULL)
    {
      itspace = new OpIterationSpaceDefinition ();
    }
    itspace->addIterationDimension (extent);
  }
}

CPPImperialOpArgMatDefinition::CPPImperialOpArgMatDefinition (
    SgExprListExp * parameters, CPPProgramDeclarationsAndDefinitions * declarations,
    OpIterationSpaceDefinition * itspace)
{
  using boost::iequals;
  using boost::lexical_cast;
  using std::string;

  SgExpressionPtrList exprs = parameters->get_expressions ();
  SgExpression * tmp;

  bool createItspace = itspace == NULL;
  tmp = exprs[indexOpMat];
  matName = isSgVarRefExp(tmp)->get_symbol ()->get_name ().getString ();

  tmp = exprs[indexOpMap1];

  map1Name = isSgVarRefExp(tmp)->get_symbol ()->get_name ().getString ();

  tmp = exprs[indexOpIndex1];

  getArgMatExtent (tmp, itspace,
      declarations->getOpMapDefinition (map1Name)->getDimension (),
      map1idx, 0, createItspace);

  tmp = exprs[indexOpMap2];

  map2Name = isSgVarRefExp(tmp)->get_symbol ()->get_name ().getString ();

  tmp = exprs[indexOpIndex2];

  getArgMatExtent (tmp, itspace,
      declarations->getOpMapDefinition (map2Name)->getDimension (),
      map2idx, 1, createItspace);

  /* Ensure that the dimensions of the iteration space are not bigger than those of the map */
  int * extent = itspace->getIterationDimensions ()[map1idx];
  int mapdim = declarations->getOpMapDefinition (map1Name)->getDimension ();
  if (extent[1] != mapdim)
  {
    Debug::getInstance ()->debugMessage (
        "OP_ARG_MAT extent of map1 index: " + lexical_cast <string> (extent[0])
        + "--" + lexical_cast <string> (extent[1])
        + " does not match dimension of map: "
        + lexical_cast <string> (mapdim),
        Debug::LOWEST_DEBUG_LEVEL, __FILE__, __LINE__);
    ROSE_ASSERT (false);
  }

  extent = itspace->getIterationDimensions ()[map2idx];
  mapdim = declarations->getOpMapDefinition (map2Name)->getDimension ();
  if (extent[1] != mapdim)
  {
    Debug::getInstance ()->debugMessage (
        "OP_ARG_MAT extent of map2 index: " + lexical_cast <string> (extent[0])
        + "--" + lexical_cast <string> (extent[1])
        + " does not match dimension of map: "
        + lexical_cast <string> (mapdim),
        Debug::LOWEST_DEBUG_LEVEL, __FILE__, __LINE__);
    ROSE_ASSERT (false);
  }

  dimension = declarations->getOpMatDefinition (matName)->getDimension ();

  tmp = exprs[indexAccessDescriptor];

  if (isSgEnumVal (tmp))
  {
    string access = isSgEnumVal (tmp)->get_name ().getString ();

    if (iequals(access, OP2::OP_INC))
    {
      accessType = OP2::OP_INC;
    }
    else if (iequals(access, OP2::OP_WRITE))
    {
      accessType = OP2::OP_WRITE;
    }
    else
    {
      Debug::getInstance ()->debugMessage (
          "Invalid OP_ARG_MAT access descriptor: " + access,
          Debug::LOWEST_DEBUG_LEVEL, __FILE__, __LINE__);
      ROSE_ASSERT (false);
    }
  }

  Debug::getInstance ()->debugMessage (
      "Found OP_ARG_MAT: accessing '" + matName + "' through maps (" + map1Name
      + ", " + map2Name + ") with dimension "
      + lexical_cast <string> (dimension),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

CPPOxfordOpSparsityDefinition::CPPOxfordOpSparsityDefinition (
    SgExprListExp * parameters, std::string const & variableName)
{
  using std::string;
  this->variableName = variableName;

  map1Name = isSgVarRefExp (
      parameters->get_expressions ()[indexOpMap1])->get_symbol ()->
      get_name ().getString ();

  map2Name = isSgVarRefExp (
      parameters->get_expressions ()[indexOpMap2])->get_symbol ()->
      get_name ().getString ();

  ROSE_ASSERT (variableName.empty () == false);
  ROSE_ASSERT (map1Name.empty () == false);
  ROSE_ASSERT (map2Name.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_SPARSITY declaration: '"
      + variableName + "' using maps " + map1Name + " and " + map2Name,
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

CPPOxfordOpMatDefinition::CPPOxfordOpMatDefinition (
    SgExprListExp * parameters, std::string const & variableName)
{
  using std::string;
  using boost::iequals;
  using boost::lexical_cast;

  this->variableName = variableName;

  sparsityName = isSgVarRefExp (
      parameters->get_expressions ()[indexOpSparsity])->get_symbol ()->
      get_name ().getString ();

  dimension = isSgIntVal (
      parameters->get_expressions ()[indexDimension])->get_value ();

  string type = isSgStringVal (
      isSgCastExp (
          parameters->get_expressions ()[indexTypeName])->get_operand ())->get_value ();

  /*
   * FIXME: is there a better way to derive the base type?
   */
  if (iequals (type, "float"))
  {
    baseType = new SgTypeFloat ();
  }
  else if (iequals(type, "double"))
  {
    baseType = new SgTypeDouble ();
  }
  else
  {
    Debug::getInstance ()->debugMessage (
        "Unknown OP_MAT entry type '" + type + "', aborting",
        Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);
    ROSE_ASSERT (false);
  }

  Debug::getInstance ()->debugMessage (
      "Found an OP_MAT declaration: it has a sparsity pattern given by '" +
      sparsityName + "', dimension " + lexical_cast <string> (dimension) +
      " and stores entries of type " + baseType->class_name (),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

CPPOxfordOpArgMatDefinition::CPPOxfordOpArgMatDefinition (
    SgExprListExp * parameters, CPPProgramDeclarationsAndDefinitions * declarations,
    OpIterationSpaceDefinition * itspace)
{
  using boost::iequals;
  using boost::lexical_cast;
  using std::string;

  SgExpressionPtrList exprs = parameters->get_expressions ();
  SgExpression * tmp;
  bool createItspace = itspace == NULL;

  tmp = exprs[indexOpMat];
  matName = isSgVarRefExp(tmp)->get_symbol ()->get_name ().getString ();


  tmp = exprs[indexOpMap1];

  map1Name = isSgVarRefExp(tmp)->get_symbol ()->get_name ().getString ();

  tmp = exprs[indexOpIndex1];

  getArgMatExtent (tmp, itspace,
      declarations->getOpMapDefinition (map1Name)->getDimension (),
      map1idx, 0, createItspace);

  tmp = exprs[indexOpMap2];

  map2Name = isSgVarRefExp(tmp)->get_symbol ()->get_name ().getString ();

  tmp = exprs[indexOpIndex2];

  getArgMatExtent (tmp, itspace,
      declarations->getOpMapDefinition (map2Name)->getDimension (),
      map2idx, 1, createItspace);

  /* Ensure that the dimensions of the iteration space are not bigger than those of the map */
  int * extent = itspace->getIterationDimensions ()[map1idx];
  int mapdim = declarations->getOpMapDefinition (map1Name)->getDimension ();
  if (extent[1] > mapdim)
  {
    Debug::getInstance ()->debugMessage (
        "OP_ARG_MAT extent of map1 index: " + lexical_cast <string> (extent[0])
        + "--" + lexical_cast <string> (extent[1])
        + " does not match dimension of map: "
        + lexical_cast <string> (mapdim),
        Debug::LOWEST_DEBUG_LEVEL, __FILE__, __LINE__);
    ROSE_ASSERT (false);
  }

  extent = itspace->getIterationDimensions ()[map2idx];
  mapdim = declarations->getOpMapDefinition (map2Name)->getDimension ();
  if (extent[1] > mapdim)
  {
    Debug::getInstance ()->debugMessage (
        "OP_ARG_MAT extent of map2 index: " + lexical_cast <string> (extent[0])
        + "--" + lexical_cast <string> (extent[1])
        + " does not match dimension of map: "
        + lexical_cast <string> (mapdim),
        Debug::LOWEST_DEBUG_LEVEL, __FILE__, __LINE__);
    ROSE_ASSERT (false);
  }

  tmp = exprs[indexDimension];

  dimension = isSgIntVal (tmp)->get_value ();

  tmp = exprs[indexAccessDescriptor];

  if (isSgEnumVal (tmp))
  {
    string access = isSgEnumVal (tmp)->get_name ().getString ();

    if (iequals(access, OP2::OP_INC))
    {
      accessType = OP2::OP_INC;
    }
    else if (iequals(access, OP2::OP_WRITE))
    {
      accessType = OP2::OP_WRITE;
    }
    else
    {
      Debug::getInstance ()->debugMessage (
          "Invalid OP_ARG_MAT access descriptor: " + access,
          Debug::LOWEST_DEBUG_LEVEL, __FILE__, __LINE__);
      ROSE_ASSERT (false);
    }
  }

  Debug::getInstance ()->debugMessage (
      "Found OP_ARG_MAT: accessing '" + matName + "' through maps (" + map1Name
      + ", " + map2Name + ") with dimension "
      + lexical_cast <string> (dimension),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}
