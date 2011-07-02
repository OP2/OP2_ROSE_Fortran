#include <CPPOP2Definitions.h>
#include <boost/lexical_cast.hpp>
#include <Debug.h>

CPPImperialOpDatDefinition::CPPImperialOpDatDefinition (
    SgExpressionPtrList & parameters, std::string const & variableName)
{
  using boost::lexical_cast;
  using std::string;

  this->variableName = variableName;

  opSetName
      = isSgVarRefExp (parameters[index_OpSetName])->get_symbol ()->get_name ().getString ();

  dimension = isSgIntVal (parameters[index_dimension])->get_value ();

  primitiveType = isSgVarRefExp (parameters[index_data])->get_type ();

  ROSE_ASSERT (opSetName.empty () == false);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (primitiveType != NULL);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_DAT declaration: '"
      + variableName + "'. The data pertains to the set '" + opSetName
      + "'. Its actual type is " + primitiveType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension), 5);
}

CPPImperialOpSetDefinition::CPPImperialOpSetDefinition (
    SgExpressionPtrList & parameters, std::string const & variableName)
{
  this->variableName = variableName;

  dimensionName
      = isSgVarRefExp (parameters[index_setCardinalityName])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (dimensionName.empty () == false);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_SET declaration: '"
      + variableName + "'. Its dimension is contained in '" + dimensionName
      + "'", 5);
}

CPPImperialOpMapDefinition::CPPImperialOpMapDefinition (
    SgExpressionPtrList & parameters, std::string const & variableName)
{
  using boost::lexical_cast;
  using std::string;

  this->variableName = variableName;

  sourceOpSetName
      = isSgVarRefExp (parameters[index_sourceOpSetName])->get_symbol ()->get_name ().getString ();

  destinationOpSetName
      = isSgVarRefExp (parameters[index_destinationOpSetName])->get_symbol ()->get_name ().getString ();

  dimension = isSgIntVal (parameters[index_setCardinality])->get_value ();

  mappingName
      = isSgVarRefExp (parameters[index_mappingName])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (sourceOpSetName.empty () == false);
  ROSE_ASSERT (destinationOpSetName.empty () == false);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (mappingName.empty () == false);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_MAP declaration: '"
      + variableName + "'. Mapping from '" + sourceOpSetName + "' to '"
      + destinationOpSetName + "' through '" + mappingName + "' with "
      + lexical_cast <string> (dimension) + " mappings per element", 5);
}

CPPImperialOpConstDefinition::CPPImperialOpConstDefinition (
    SgExpressionPtrList & parameters)
{
  using boost::lexical_cast;
  using std::string;

  dimension = isSgIntVal (parameters[index_dimension])->get_value ();

  SgAddressOfOp * addressOfOperator = isSgAddressOfOp (
      parameters[index_OpDatName]);

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
        = isSgVarRefExp (parameters[index_OpDatName])->get_symbol ()->get_name ().getString ();
  }

  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_CONST declaration: '"
      + variableName + "' Its dimension is "
      + lexical_cast <string> (dimension), 5);
}
