#include <RoseHelper.h>

Sg_File_Info * RoseHelper::fileInfo;

Sg_File_Info *
RoseHelper::getFileInfo ()
{
  if (fileInfo == NULL)
  {
    fileInfo = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode ();
    fileInfo->setOutputInCodeGeneration ();
  }
  return fileInfo;
}

std::string const
RoseHelper::getFirstVariableName (SgVariableDeclaration * variableDeclaration)
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
