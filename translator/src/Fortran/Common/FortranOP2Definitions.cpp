#include <FortranOP2Definitions.h>
#include <Debug.h>
#include <boost/lexical_cast.hpp>
#include <rose.h>

FortranOpDatDefinition::FortranOpDatDefinition (SgExprListExp * parameters)
{
  using boost::lexical_cast;
  using std::string;

  /*
   * ======================================================
   * Get name of OP_SET
   * ======================================================
   */

  SgVarRefExp * opSetVariableReference;

  if (isSgDotExp (parameters->get_expressions ()[index_OpSetName]) != NULL)
  {
    opSetVariableReference = isSgVarRefExp (isSgDotExp (
        parameters->get_expressions ()[index_OpSetName])->get_rhs_operand ());
  }
  else
  {
    opSetVariableReference = isSgVarRefExp (
        parameters->get_expressions ()[index_OpSetName]);
  }

  opSetName = opSetVariableReference->get_symbol ()->get_name ().getString ();

  /*
   * ======================================================
   * Get dimension of OP_DAT
   * ======================================================
   */

  dimension
      = isSgIntVal (parameters->get_expressions ()[index_dimension])->get_value ();

  /*
   * ======================================================
   * Get type of OP_DAT from the actual data declaration
   * it wraps
   * ======================================================
   */

  SgVarRefExp * actualDataVariableReference;

  if (isSgDotExp (parameters->get_expressions ()[index_data]) != NULL)
  {
    actualDataVariableReference = isSgVarRefExp (isSgDotExp (
        parameters->get_expressions ()[index_data])->get_rhs_operand ());
  }
  else
  {
    actualDataVariableReference = isSgVarRefExp (
        parameters->get_expressions ()[index_data]);
  }

  baseType = actualDataVariableReference->get_type ();

  /*
   * ======================================================
   * Get name of OP_DAT
   * ======================================================
   */

  SgVarRefExp * opDatVariableReference;

  if (isSgDotExp (parameters->get_expressions ()[index_OpDatName]) != NULL)
  {
    opDatVariableReference = isSgVarRefExp (isSgDotExp (
        parameters->get_expressions ()[index_OpDatName])->get_rhs_operand ());
  }
  else
  {
    opDatVariableReference = isSgVarRefExp (
        parameters->get_expressions ()[index_OpDatName]);
  }

  variableName
      = opDatVariableReference->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (opSetName.empty () == false);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (baseType != NULL);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_DAT definition: '"
      + variableName + "'. The data pertains to the set '" + opSetName
      + "'. Its actual type is " + baseType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

FortranOpSetDefinition::FortranOpSetDefinition (SgExprListExp * parameters)
{
  /*
   * ======================================================
   * Get name of OP_SET dimension variable
   * ======================================================
   */

  if (isSgPntrArrRefExp (
      parameters->get_expressions ()[index_setCardinalityName]) != NULL)
  {
    dimensionName
        = isSgPntrArrRefExp (
            parameters->get_expressions ()[index_setCardinalityName])->unparseToString ();
  }
  else
  {
    dimensionName
        = isSgVarRefExp (
            parameters->get_expressions ()[index_setCardinalityName])->get_symbol ()->get_name ().getString ();
  }

  /*
   * ======================================================
   * Get name of OP_SET
   * ======================================================
   */

  SgVarRefExp * opSetVariableReference;

  if (isSgDotExp (parameters->get_expressions ()[index_OpSetName]) != NULL)
  {
    opSetVariableReference = isSgVarRefExp (isSgDotExp (
        parameters->get_expressions ()[index_OpSetName])->get_rhs_operand ());
  }
  else
  {
    opSetVariableReference = isSgVarRefExp (
        parameters->get_expressions ()[index_OpSetName]);
  }

  variableName
      = opSetVariableReference->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (dimensionName.empty () == false);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_SET definition: '"
      + variableName + "'. Its dimension is contained in '" + dimensionName
      + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

FortranOpMapDefinition::FortranOpMapDefinition (SgExprListExp * parameters)
{
  using boost::lexical_cast;
  using std::string;

  /*
   * ======================================================
   * Get name of source OP_SET
   * ======================================================
   */

  SgVarRefExp * opSetSourceVariableReference;

  if (isSgDotExp (parameters->get_expressions ()[index_sourceOpSetName])
      != NULL)
  {
    opSetSourceVariableReference
        = isSgVarRefExp (
            isSgDotExp (parameters->get_expressions ()[index_sourceOpSetName])->get_rhs_operand ());
  }
  else
  {
    opSetSourceVariableReference = isSgVarRefExp (
        parameters->get_expressions ()[index_sourceOpSetName]);
  }

  sourceOpSetName
      = opSetSourceVariableReference->get_symbol ()->get_name ().getString ();

  /*
   * ======================================================
   * Get name of destination OP_SET
   * ======================================================
   */

  SgVarRefExp * opSetDestinationVariableReference;

  if (isSgDotExp (parameters->get_expressions ()[index_destinationOpSetName])
      != NULL)
  {
    opSetDestinationVariableReference
        = isSgVarRefExp (
            isSgDotExp (
                parameters->get_expressions ()[index_destinationOpSetName])->get_rhs_operand ());
  }
  else
  {
    opSetDestinationVariableReference = isSgVarRefExp (
        parameters->get_expressions ()[index_destinationOpSetName]);
  }

  destinationOpSetName
      = opSetDestinationVariableReference->get_symbol ()->get_name ().getString ();

  /*
   * ======================================================
   * Get number of elements per mapping
   * ======================================================
   */

  dimension
      = isSgIntVal (parameters->get_expressions ()[index_dimension])->get_value ();

  /*
   * ======================================================
   * Get dimension of
   * ======================================================
   */

  SgVarRefExp * mappingCardinalityVariableReference;

  if (isSgDotExp (parameters->get_expressions ()[index_mappingCardinalityName])
      != NULL)
  {
    mappingCardinalityVariableReference
        = isSgVarRefExp (
            isSgDotExp (
                parameters->get_expressions ()[index_mappingCardinalityName])->get_rhs_operand ());
  }
  else
  {
    mappingCardinalityVariableReference = isSgVarRefExp (
        parameters->get_expressions ()[index_mappingCardinalityName]);
  }

  mappingCardinalityName
      = mappingCardinalityVariableReference->get_symbol ()->get_name ().getString ();

  /*
   * ======================================================
   * Get mapping name
   * ======================================================
   */

  SgVarRefExp * mappingVariableReference;

  if (isSgDotExp (parameters->get_expressions ()[index_mappingName]) != NULL)
  {
    mappingVariableReference = isSgVarRefExp (isSgDotExp (
        parameters->get_expressions ()[index_mappingName])->get_rhs_operand ());
  }
  else
  {
    mappingVariableReference = isSgVarRefExp (
        parameters->get_expressions ()[index_mappingName]);
  }

  mappingName
      = mappingVariableReference->get_symbol ()->get_name ().getString ();

  /*
   * ======================================================
   * Get name of OP_MAP
   * ======================================================
   */

  variableName
      = isSgVarRefExp (parameters->get_expressions ()[index_OpMapName])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (sourceOpSetName.empty () == false);
  ROSE_ASSERT (destinationOpSetName.empty () == false);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (mappingCardinalityName.empty () == false);
  ROSE_ASSERT (mappingName.empty () == false);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_MAP definition: '"
      + variableName + "'. Mapping from '" + sourceOpSetName + "' to '"
      + destinationOpSetName + "' through '" + mappingName
      + "' (whose cardinality is set in '" + mappingCardinalityName
      + "') with " + lexical_cast <string> (dimension)
      + " mappings per element", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

FortranOpGblDefinition::FortranOpGblDefinition (SgExprListExp * parameters)
{
  using boost::lexical_cast;
  using std::string;

  baseType
      = isSgVarRefExp (parameters->get_expressions ()[index_data])->get_type ();

  variableName
      = isSgVarRefExp (parameters->get_expressions ()[index_OpDatName])->get_symbol ()->get_name ().getString ();

  dimension
      = isSgIntVal (parameters->get_expressions ()[index_dimension])->get_value ();

  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (baseType != NULL);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_GBL definition: '"
      + variableName + "'. Its actual type is " + baseType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

FortranOpGblScalarDefinition::FortranOpGblScalarDefinition (
    SgExprListExp * parameters)
{
  using boost::lexical_cast;
  using std::string;

  baseType
      = isSgVarRefExp (parameters->get_expressions ()[index_data])->get_type ();

  variableName
      = isSgVarRefExp (parameters->get_expressions ()[index_OpDatName])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (baseType != NULL);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_GBL scalar definition: '"
      + variableName + "'. Its actual type is " + baseType->class_name (),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

FortranOpConstDefinition::FortranOpConstDefinition (SgExprListExp * parameters,
    SgFunctionCallExp * callExpression) :
  callExpression (callExpression)
{
  using boost::lexical_cast;
  using std::string;

  dimension
      = isSgIntVal (parameters->get_expressions ()[index_dimension])->get_value ();

  variableName
      = isSgVarRefExp (parameters->get_expressions ()[index_OpConstName])->get_symbol ()->get_name ().getString ();

  baseType
      = isSgVarRefExp (parameters->get_expressions ()[index_OpConstName])->get_type ();

  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_CONST definition: '"
      + variableName + "'. Its dimension is " + lexical_cast <string> (
      dimension) + " and its type is " + baseType->class_name (),
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

SgFunctionCallExp *
FortranOpConstDefinition::getCallSite ()
{
  return callExpression;
}
