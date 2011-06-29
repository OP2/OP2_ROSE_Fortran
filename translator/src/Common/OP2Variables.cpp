#include <boost/lexical_cast.hpp>
#include <OP2Variables.h>
#include <Debug.h>

OpDatDeclaration::OpDatDeclaration (SgExpressionPtrList & parameters)
{
  using boost::lexical_cast;
  using std::string;

  dimension = isSgIntVal (parameters[index_dimension])->get_value ();

  /*
   * ======================================================
   * The 'inputData' parameter to OP_DECL_DAT holds the base
   * type of the OP_DAT variable
   * ======================================================
   */
  actualType = isSgVarRefExp (parameters[index_inputData])->get_type ();

  variableName
      = isSgVarRefExp (parameters[index_OP_DAT])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (actualType != NULL);
  ROSE_ASSERT (variableName != "");

  Debug::getInstance ()->debugMessage ("'" + variableName
      + "' is an OP_DAT. Its actual type is " + actualType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension), 8);
}

OpSetDeclaration::OpSetDeclaration (SgExpressionPtrList & parameters)
{
  variableName
      = isSgVarRefExp (parameters[index_OP_SET])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (variableName != "");

  Debug::getInstance ()->debugMessage (variableName + " is an OP_SET", 8);
}

OpMapDeclaration::OpMapDeclaration (SgExpressionPtrList & parameters)
{
  variableName
      = isSgVarRefExp (parameters[index_OP_MAP])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (variableName != "");

  Debug::getInstance ()->debugMessage (variableName + " is an OP_MAP", 8);
}

OpGblDeclaration::OpGblDeclaration (SgExpressionPtrList & parameters)
{
  using boost::lexical_cast;
  using std::string;

  dimension = isSgIntVal (parameters[index_dimension])->get_value ();

  /*
   * ======================================================
   * The 'inputData' parameter to OP_DECL_GBL holds the base
   * type of the OP_DAT variable
   * ======================================================
   */
  actualType = isSgVarRefExp (parameters[index_inputData])->get_type ();

  variableName
      = isSgVarRefExp (parameters[index_OP_DAT])->get_symbol ()->get_name ().getString ();

  ROSE_ASSERT (dimension > 0);
  ROSE_ASSERT (actualType != NULL);
  ROSE_ASSERT (variableName != "");

  Debug::getInstance ()->debugMessage ("'" + variableName
      + "' is an OP_GBL. Its actual type is " + actualType->class_name ()
      + " and its dimension is " + lexical_cast <string> (dimension), 8);
}
