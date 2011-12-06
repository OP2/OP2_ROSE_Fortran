#include "RoseHelper.h"
#include "Debug.h"
#include <rose.h>

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

void
RoseHelper::forceOutputOfCodeToFile (SgNode * root)
{
  Debug::getInstance ()->debugMessage (
      "Forcing output of syntax tree in output file", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

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
