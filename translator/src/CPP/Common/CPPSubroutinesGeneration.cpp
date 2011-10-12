#include <CPPSubroutinesGeneration.h>
#include <CPPOpDatDimensionsDeclaration.h>
#include <CPPParallelLoop.h>
#include <CPPHostSubroutine.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <Exceptions.h>

void
CPPSubroutinesGeneration::patchCallsToParallelLoops ()
{
  using std::map;
  using std::string;
  using std::vector;
  using SageBuilder::buildFunctionRefExp;

  Debug::getInstance ()->debugMessage ("Patching calls to OP_PAR_LOOPs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <string> processedFiles;

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    CPPParallelLoop * parallelLoop =
        static_cast <CPPParallelLoop *> (it->second);

    CPPHostSubroutine * hostSubroutine = hostSubroutines[userSubroutineName];

    SgFunctionCallExp * functionCallExpression =
        parallelLoop->getFunctionCall ();

    SgScopeStatement * scope = isSgExprStatement (
        functionCallExpression->get_parent ())->get_scope ();

    SgFunctionRefExp * hostSubroutineReference = buildFunctionRefExp (
        hostSubroutine->getSubroutineHeaderStatement ());

    functionCallExpression->set_function (hostSubroutineReference);

    /*
     * ==================================================
     * Remove the first parameter (kernel reference)
     * ==================================================
     */

    SgExpressionPtrList & arguments =
        functionCallExpression->get_args ()->get_expressions ();

    arguments.erase (arguments.begin ());

  }
}

SgSourceFile &
CPPSubroutinesGeneration::createSourceFile ()
{
  using SageBuilder::buildFile;
  using std::string;

  Debug::getInstance ()->debugMessage ("Generating file '" + newFileName + "'",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  FILE * inputFile = fopen (newFileName.c_str (), "w+");
  if (inputFile != NULL)
  {
    Debug::getInstance ()->debugMessage ("Creating dummy source file '"
        + newFileName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    fclose (inputFile);
  }
  else
  {
    throw Exceptions::CodeGeneration::FileCreationException (
        "Could not create dummy Fortran file '" + newFileName + "'");
  }

  SgSourceFile * sourceFile = isSgSourceFile (buildFile (newFileName,
      newFileName, NULL));

  sourceFile->set_Cxx_only (true);

  return *sourceFile;
}

void
CPPSubroutinesGeneration::generate ()
{
  SgSourceFile & sourceFile = createSourceFile ();

  moduleScope = sourceFile.get_globalScope ();

  addLibraries ();

  createSubroutines ();

  patchCallsToParallelLoops ();
}

