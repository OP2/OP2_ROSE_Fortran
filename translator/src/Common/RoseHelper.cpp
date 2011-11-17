#include <RoseHelper.h>
#include "Debug.h"

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

void
RoseHelper::forceOutputOfCodeToFile (SgNode * root)
{
  Debug::getInstance ()->debugMessage (
      "Forcing output of syntax tree in output file", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * We have to set each node in the AST representation of
   * this subroutine as compiler generated, otherwise chunks
   * of the user kernel are missing in the output
   * ======================================================
   */

  class TreeVisitor: public AstSimpleProcessing
  {
    public:

      TreeVisitor ()
      {
      }

      virtual void
      visit (SgNode * node)
      {
        SgLocatedNode * locatedNode = isSgLocatedNode (node);
        if (locatedNode != NULL)
        {
          locatedNode->setOutputInCodeGeneration ();
        }
      }
  };

  TreeVisitor * visitor = new TreeVisitor ();

  visitor->traverse (root, preorder);
}
