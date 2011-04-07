#include <ROSEHelper.h>

/*
 * ======================================================
 * Allocation of class-wide static variables
 * ======================================================
 */

Sg_File_Info * ROSEHelper::fileInfo;

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

Sg_File_Info *
ROSEHelper::getFileInfo ()
{
  if (fileInfo == NULL)
  {
    fileInfo = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode ();
    fileInfo->setOutputInCodeGeneration ();
  }
  return fileInfo;
}

std::string const
ROSEHelper::getFirstVariableName (SgVariableDeclaration * variableDeclaration)
{
  SgInitializedNamePtrList & variables = variableDeclaration->get_variables ();

  /*
   * ======================================================
   * Only handle the case where the declaration contains
   * exactly one variable
   * ======================================================
   */
  ROSE_ASSERT (variables.size() == 1);

  return variables.front ()->get_name ().getString ();
}
