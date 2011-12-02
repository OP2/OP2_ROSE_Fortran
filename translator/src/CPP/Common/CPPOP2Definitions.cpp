#include <CPPOP2Definitions.h>
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
