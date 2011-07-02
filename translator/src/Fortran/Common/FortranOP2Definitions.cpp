#include <FortranOP2Definitions.h>
#include <boost/lexical_cast.hpp>
#include <Debug.h>

FortranOpDatDefinition::FortranOpDatDefinition (
    SgExpressionPtrList & parameters)
{
  using boost::lexical_cast;
  using std::string;

  opSetName
      = isSgVarRefExp (parameters[index_OpSetName])->get_symbol ()->get_name ().getString ();

  dimension = isSgIntVal (parameters[index_dimension])->get_value ();

  primitiveType = isSgVarRefExp (parameters[index_data])->get_type ();

  variableName
      = isSgVarRefExp (parameters[index_OpDatName])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (opSetName.empty () == false);
  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (primitiveType != NULL);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_DAT definition: '"
      + variableName + "'. The data pertains to the set '" + opSetName
      + "'. Its actual type is " + primitiveType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension), 5);
}

FortranOpSetDefinition::FortranOpSetDefinition (
    SgExpressionPtrList & parameters)
{
  dimensionName
      = isSgVarRefExp (parameters[index_setCardinalityName])->get_symbol ()->get_name ().getString ();

  variableName
      = isSgVarRefExp (parameters[index_OpSetName])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (dimensionName.empty () == false);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_SET definition: '"
      + variableName + "'. Its dimension is contained in '" + dimensionName
      + "'", 5);
}

FortranOpMapDefinition::FortranOpMapDefinition (
    SgExpressionPtrList & parameters)
{
  using boost::lexical_cast;
  using std::string;

  sourceOpSetName
      = isSgVarRefExp (parameters[index_sourceOpSetName])->get_symbol ()->get_name ().getString ();

  destinationOpSetName
      = isSgVarRefExp (parameters[index_destinationOpSetName])->get_symbol ()->get_name ().getString ();

  dimension = isSgIntVal (parameters[index_dimension])->get_value ();

  mappingCardinalityName
      = isSgVarRefExp (parameters[index_mappingCardinalityName])->get_symbol ()->get_name ().getString ();

  mappingName
      = isSgVarRefExp (parameters[index_mappingName])->get_symbol ()->get_name ().getString ();

  variableName
      = isSgVarRefExp (parameters[index_OpMapName])->get_symbol ()->get_name ().getString ();

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
      + " mappings per element", 5);
}

FortranOpGblDefinition::FortranOpGblDefinition (
    SgExpressionPtrList & parameters)
{
  using boost::lexical_cast;
  using std::string;

  primitiveType = isSgVarRefExp (parameters[index_data])->get_type ();

  dimension = isSgIntVal (parameters[index_dimension])->get_value ();

  variableName
      = isSgVarRefExp (parameters[index_OpDatName])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (primitiveType != NULL);
  ROSE_ASSERT (variableName.empty () == false);

  Debug::getInstance ()->debugMessage ("Found an OP_GBL definition: '"
      + variableName + "'. Its actual type is " + primitiveType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension), 5);
}
