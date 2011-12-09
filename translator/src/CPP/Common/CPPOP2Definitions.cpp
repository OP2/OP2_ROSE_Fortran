#include <CPPOP2Definitions.h>
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

  primitiveType
      = isSgVarRefExp (parameters->get_expressions ()[indexDataArray])->get_type ();

  ROSE_ASSERT (opSetName.empty () == false);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (primitiveType != NULL);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_DAT declaration: '"
      + variableName + "'. The data pertains to the set '" + opSetName
      + "'. Its actual type is " + primitiveType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

CPPImperialOpSetDefinition::CPPImperialOpSetDefinition (
    SgExprListExp * parameters, std::string const & variableName)
{
  this->variableName = variableName;

  dimensionName
      = isSgVarRefExp (parameters->get_expressions ()[indexDimension])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (dimensionName.empty () == false);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_SET declaration: '"
      + variableName + "'. Its dimension is contained in '" + dimensionName
      + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__ );
}

SgNode*
CPPImperialOpSubSetDefinition::getOpArgDat(int i) {
    return this->parameters->get_expressions ()[2 + i];
}

CPPImperialOpSubSetDefinition::CPPImperialOpSubSetDefinition (
                                                        SgExprListExp * parameters, std::string const & variableName)
{
    this->variableName = variableName;
    
    this->originSetName = isSgVarRefExp (parameters->get_expressions ()[indexOriginSet])->get_symbol ()->get_name ().getString ();
    
    this->filterKernelName = isSgFunctionRefExp (parameters->get_expressions ()[indexFilterKernel])->get_symbol ()->get_name ().getString ();
    
    this->nbFilterArg = parameters->get_expressions ().size () - 2;
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

  primitiveType
      = isSgVarRefExp (parameters->get_expressions ()[indexDataArray])->get_type ();

  ROSE_ASSERT (opSetName.empty () == false);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (primitiveType != NULL);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_DAT declaration: '"
      + variableName + "'. The data pertains to the set '" + opSetName
      + "'. Its actual type is " + primitiveType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

CPPOxfordOpSetDefinition::CPPOxfordOpSetDefinition (SgExprListExp * parameters,
    std::string const & variableName)
{
  this->variableName = variableName;

  dimensionName
      = isSgVarRefExp (parameters->get_expressions ()[indexDimension])->get_symbol ()->get_name ().getString ();

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

CPPOxfordOpConstDefinition::CPPOxfordOpConstDefinition (
    SgExprListExp * parameters)
{
  using boost::lexical_cast;
  using std::string;

  dimension
      = isSgIntVal (parameters->get_expressions ()[indexDimension])->get_value ();

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
