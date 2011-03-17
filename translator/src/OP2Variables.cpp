#include <boost/lexical_cast.hpp>

#include "OP2Variables.h"
#include "Debug.h"

OP_DAT_Declaration::OP_DAT_Declaration (SgExpressionPtrList & parameters)
{
  using boost::lexical_cast;
  using std::string;

  dimension = isSgIntVal (parameters[index_dimension])->get_value ();
  actualType = isSgVarRefExp (parameters[index_OP_DAT])->get_type ();

  variableName
      = isSgVarRefExp (parameters[index_OP_DAT])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (actualType != NULL);

  Debug::getInstance ()->debugMessage ("'" + variableName
      + "' is an OP_DAT. Its actual type is " + actualType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension), 8);
}

OP_SET_Declaration::OP_SET_Declaration (SgExpressionPtrList & parameters)
{
  variableName
      = isSgVarRefExp (parameters[index_OP_SET])->get_symbol ()->get_name ().getString ();

  Debug::getInstance ()->debugMessage (variableName + " is an OP_SET", 8);
}

OP_MAP_Declaration::OP_MAP_Declaration (SgExpressionPtrList & parameters)
{
  variableName
      = isSgVarRefExp (parameters[index_OP_MAP])->get_symbol ()->get_name ().getString ();

  Debug::getInstance ()->debugMessage (variableName + " is an OP_MAP", 8);
}

OP_GBL_Declaration::OP_GBL_Declaration (SgExpressionPtrList & parameters)
{
  using boost::lexical_cast;
  using std::string;

  dimension = isSgIntVal (parameters[index_dimension])->get_value ();
  actualType = isSgVarRefExp (parameters[index_OP_DAT])->get_type ();

  variableName
      = isSgVarRefExp (parameters[index_OP_DAT])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (actualType != NULL);

  Debug::getInstance ()->debugMessage ("'" + variableName
      + "' is an OP_GBL. Its actual type is " + actualType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension), 8);
}
